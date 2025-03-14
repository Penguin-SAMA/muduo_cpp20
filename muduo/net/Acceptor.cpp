#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include "Socket.h"
#include <ctime>
#include <sys/epoll.h>

namespace muduo {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback([this]() { handleRead(); });
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.setEvents(EPOLLIN);
    loop_->updateChannel(&acceptChannel_);
}

void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("Acceptor::handleRead accept error");
    }
}

}
} // namespace muduo::net
