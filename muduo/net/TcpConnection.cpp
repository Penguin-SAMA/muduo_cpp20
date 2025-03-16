#include "TcpConnection.h"
#include "Channel.h"
#include "Logging.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <sys/epoll.h>
#include <sys/types.h>
#include <system_error>
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
        if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
            ssize_t n = ::write(channel_->fd(), message.data(), message.size());
            if (n >= 0) {
                size_t remaining = message.size() - n;
                if (remaining > 0) {
                    outputBuffer_.append(message.data() + n, remaining);
                    channel_->enableWriting();
                    loop_->updateChannel(channel_.get());
                } else {
                }
            } else {
                outputBuffer_.append(message.data(), message.size());
                channel_->enableWriting();
                loop_->updateChannel(channel_.get());
            }
        } else {
            outputBuffer_.append(message.data(), message.size());
        }
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
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        std::string msg(inputBuffer_.peek(), inputBuffer_.readableBytes());
        inputBuffer_.retrieveAll();
        if (messageCallback_) {
            messageCallback_(shared_from_this(), msg);
        }
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                loop_->updateChannel(channel_.get());
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite error: {}", strerror(errno));
        }
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
