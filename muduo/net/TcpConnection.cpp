#include "TcpConnection.h"
#include "Channel.h"
#include "Logging.h"
#include <memory>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

namespace muduo {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                             const InetAddress& localAddr, const InetAddress& peerAddr)
    : loop_(loop)
    , name_(name)
    , state_(kConnecting)
    , socket_(std::make_unique<Socket>(sockfd))
    , channel_(std::make_unique<Channel>(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr) {
    channel_->setReadCallback([this]() { handleRead(); });
    channel_->setWriteCallback([this]() { handleWrite(); });
    channel_->setCloseCallback([this]() { handleClose(); });
    channel_->setErrorCallback([this]() { handleError(); });

    LOG_INFO("TcpConnection::ctor [{}] created.", name_);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor [{}] destroyed.", name_);
}

void TcpConnection::send(const std::string& message) {
    if (state_ == kConnected) {
        outputBuffer_.append(message);
        channel_->setEvents(EPOLLIN | EPOLLOUT);
        loop_->updateChannel(channel_.get());
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->setEvents(EPOLLIN);
    loop_->updateChannel(channel_.get());
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        loop_->removeChannel(channel_.get());
    }
}

void TcpConnection::handleRead() {
    char buf[4096];
    ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    if (n > 0) {
        inputBuffer_.append(buf, n);
        if (messageCallback_) {
            messageCallback_(shared_from_this(), inputBuffer_);
            inputBuffer_.clear();
        }
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.data(), outputBuffer_.size());
    if (n > 0) {
        outputBuffer_ = outputBuffer_.substr(n);
        if (outputBuffer_.empty()) {
            channel_->setEvents(EPOLLIN);
            loop_->updateChannel(channel_.get());
        }
    } else {
        LOG_ERROR("TcpConnection::handleWrite error");
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    if (closeCallback_) {
        closeCallback_(shared_from_this());
    }
}

void TcpConnection::handleError() {
    LOG_ERROR("TcpConnection::handleError: socket error");
}

}
} // namespace muduo::net
