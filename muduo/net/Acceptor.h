#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"
#include <functional>

namespace muduo {
namespace net {

class EventLoop;

// @brief 封装TCP listen socket + Channel
//        用于接受新连接
class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& peerAddr)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = false);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    //  有新连接到来时触发
    void handleRead();

private:
    EventLoop* loop_;                             // 所属事件循环
    Socket acceptSocket_;                         // 监听套接字
    Channel acceptChannel_;                       // 与监听套接字绑定的Channel
    NewConnectionCallback newConnectionCallback_; // 上层回调
    bool listenning_;
};

}
} // namespace muduo::net
