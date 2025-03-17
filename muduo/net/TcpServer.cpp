#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Logging.h"
#include "TcpConnection.h"
#include <format>
#include <memory>
#include <string>
#include <utility>

namespace muduo {
namespace net {
TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& name,
                     bool reusePort)
    : loop_(loop)
    , name_(name)
    , acceptor_(std::make_unique<Acceptor>(loop, listenAddr, reusePort))
    , started_(false)
    , nextConnId_(1)
    , threadNum_(0) {
    // 当 Acceptor 有新连接到达时调用
    acceptor_->setNewConnectionCallback([this](int sockfd, const InetAddress& peerAddr) {
        this->newConnection(sockfd, peerAddr);
    });

    LOG_INFO("TcpServer [{}] - ctor: listen on {}:{}", name_, listenAddr.toIp(), listenAddr.toIpPort());
}

TcpServer::~TcpServer() {
    LOG_INFO("TcpServer [{}] - dtor.", name_);
}

void TcpServer::setThreadNum(int numThreads) {
    threadNum_ = numThreads;
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;

        if (!threadPool_) {
            threadPool_ = std::make_unique<EventLoopThreadPool>(loop_, threadNum_);
            threadPool_->start();
        }

        acceptor_->listen();
        LOG_INFO("TcpServer [{}] started.", name_);
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    // 这个函数在 Acceptor 对应的 Eventloop 线程中被调用
    // 为新连接创建 TcpConnection 对象
    EventLoop* ioLoop = threadPool_->getNextLoop();

    std::string connName = std::format("{}-conn{}", name_, nextConnId_++);
    LOG_INFO("TcpServer::newConnection [{}] - new connection: {}", name_, connName);

    // 使用 shared_ptr 管理 TcpConnection
    InetAddress localAddr;
    auto conn = std::make_shared<TcpConnection>(
        loop_,     // 所属事件循环
        connName,  // 连接名称
        sockfd,    // 已经 acceptor 返回的连接 fd
        localAddr, // 服务器本地地址
        peerAddr   // 客户端地址
    );

    // 将新连接放入容器中
    connections_[connName] = conn;

    // 设置用户自定义的回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);

    // 设置关闭回调，以便在连接关闭时通知 TcpServer 移除此连接
    conn->setCloseCallback([this](const TcpConnectionPtr& c) {
        removeConnection(c);
    });

    // 建立连接
    ioLoop->runInLoop([conn] {
        conn->connectEstablished();
    });
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop([this, conn]() {
        removeConnectionInLoop(conn);
    });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [{}] - conn name: {}", name_, conn->name());

    if (!connections_.contains(conn->name())) {
        // 理论上不会发生
        LOG_WARN("removeConnectionInLoop - connection {} not found!", conn->name());
        return;
    }

    // 从map移除
    size_t n = connections_.erase(conn->name());
    LOG_INFO("TcpServer::removeConnectionInLoop [{}] - conn erased, count={}", conn->name(), n);

    // 销毁连接
    conn->connectDestroyed();
}

}
} // namespace muduo::net
