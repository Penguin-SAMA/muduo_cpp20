#pragma once

#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace muduo {
namespace net {

// TcpServer功能：
// 1. 管理Acceptor，负责listen并在有新连接到达时创建TcpConnection
// 2. 保存和管理所有活动的TcpConnection对象
// 3. 提供设置回调的接口给上层调用者
// 4. 当连接关闭时，从容地移除对应连接

class EventLoop;

class TcpServer
{
public:
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using MessageCallback = TcpConnection::MessageCallback;
    using CloseCallbace = TcpConnection::CloseCallback;

    // @param loop          : 主 Reactor，负责执行事件回调
    // @param listenAddr    : 监听地址（IP + 端口）
    // @param name          : 给当前 TcpServer 起一个名字，方便日志或调试
    // @param reusePort     : 是否开启 SO_REUSEPORT
    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name,
              bool reusePort = false);
    ~TcpServer();

    // 设置连接建立/断开的回调
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    // 设置可读事件回调
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void setThreadNum(int numThreads);

    void setIdleTimeout(std::chrono::seconds timeout) { idleTimeout_ = timeout; }
    void checkIdleConnection(const TcpConnectionPtr& conn);

    // 启动服务器
    void start();

private:
    // 有新连接到来时，Acceptor会调用此函数
    void newConnection(int sockfd, const InetAddress& peerAddr);

    // 移除连接。TcpConnection 在发生 close 时会调用此函数
    void removeConnection(const TcpConnectionPtr& conn);

    // 供 removeConnection 内部使用，将清理操作放到 Eventloop 的队列中执行
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;                    // base loop
    std::string name_;                   // 服务器名字
    std::unique_ptr<Acceptor> acceptor_; // 用于监听新连接
    bool started_;                       // 是否已经启动
    int nextConnId_;                     // 用于给新连接生成唯一名称
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    int threadNum_;

    std::chrono::seconds idleTimeout_{60};
    std::map<std::string, TimerId> connectionTimers_;

    // 回调函数
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    // 保存所有活动的连接，key是连接的name
    ConnectionMap connections_;
};

}
} // namespace muduo::net
