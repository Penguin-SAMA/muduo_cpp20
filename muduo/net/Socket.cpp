#include "Socket.h"
#include "Logging.h"
#include <asm-generic/socket.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace muduo {
namespace net {

static int setNonBlock(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    return ::fcntl(fd, F_SETFL, flags);
}

int createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_ERROR("socket failed err: {}", strerror(errno));
    }
    return sockfd;
}

Socket::~Socket() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
    }
}

void Socket::bindAddress(const InetAddress& localaddr) {
    int ret = ::bind(sockfd_,
                     reinterpret_cast<const struct sockaddr*>(&(localaddr.getSockAddrInet())),
                     static_cast<socklen_t>(sizeof(struct sockaddr_in)));

    if (ret < 0) {
        LOG_ERROR("bind error: {}", strerror(errno));
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);

    if (ret < 0) {
        LOG_ERROR("listen error: {}", strerror(errno));
    }
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet(addr);
    } else {
        LOG_ERROR("accept error: {}", strerror(errno));
    }
    return connfd;
}

void Socket::setNonBlockAndCloseOnExec() {
    // 设置非阻塞
    if (setNonBlock(sockfd_) < 0) {
        LOG_ERROR("setNonBlock error: {}", strerror(errno));
    }

    // 设置FD_CLOEXEC
    int flags = ::fcntl(sockfd_, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    if (::fcntl(sockfd_, F_SETFD, flags) < 0) {
        LOG_ERROR("set FD_CLOEXEC error: {}", strerror(errno));
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(optval));
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdown write error: {}", strerror(errno));
    }
}

}
} // namespace muduo::net
