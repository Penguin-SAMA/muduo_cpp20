#pragma once

#include "InetAddress.h"
#include <sys/socket.h>

namespace muduo {
namespace net {

// @brief 封装 socket fd 的管理及常用操作
class Socket
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }

    // 绑定地址
    void bindAddress(const InetAddress& localaddr);
    // 监听
    void listen();
    // 接受连接，返回一个新的socket fd，并把对端地址写入peeraddr
    int accept(InetAddress* peeraddr);

    // 设置非阻塞
    void setNonBlockAndCloseOnExec();

    // 打开/关闭地址复用
    void setReuseAddr(bool on);

    // 关闭写端
    void shutdownWrite();

private:
    int sockfd_;
};

// @brief 创建一个非阻塞 socket
int createNonblocking();

}
} // namespace muduo::net
