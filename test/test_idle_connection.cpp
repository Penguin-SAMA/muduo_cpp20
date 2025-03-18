
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>

// 为方便测试，我们创建一个 FakeTcpConnection 继承自 TcpConnection
// 并增加一个标志字段 shutdownCalled_，在 shutdown() 被调用时设置该标志。
class FakeTcpConnection : public muduo::net::TcpConnection
{
public:
    // 假设 TcpConnection 构造函数为：
    // TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
    //               const InetAddress& localAddr, const InetAddress& peerAddr);
    FakeTcpConnection(muduo::net::EventLoop* loop, const std::string& name, int sockfd,
                      const muduo::net::InetAddress& localAddr,
                      const muduo::net::InetAddress& peerAddr)
        : muduo::net::TcpConnection(loop, name, sockfd, localAddr, peerAddr)
        , shutdownCalled_(false) {
        // 初始化 lastReceiveTime_ 为当前时间
        lastReceiveTime_ = std::chrono::steady_clock::now();
    }

    // 重写 shutdown() 方法，设置标志
    void shutdown() override {
        shutdownCalled_ = true;
        // 可调用基类 shutdown()，或直接不调用（根据你的设计）
        muduo::net::TcpConnection::shutdown();
    }

    bool shutdownCalled() const { return shutdownCalled_; }

    // 为测试提供 setter 用于修改 lastReceiveTime_
    void setLastReceiveTime(std::chrono::steady_clock::time_point t) {
        lastReceiveTime_ = t;
    }

    // 假设 lastReceiveTime_ 为 protected 或 public，否则需要增加相应接口
    using muduo::net::TcpConnection::lastReceiveTime_;

private:
    bool shutdownCalled_;
};

// 为了能调用 TcpServer::checkIdleConnection()，我们构造一个测试用的 TcpServer 子类
// 假设 TcpServer::checkIdleConnection() 是 public 或我们将其声明为 public 供测试使用
class TestTcpServer : public muduo::net::TcpServer
{
public:
    TestTcpServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& addr, const std::string& name)
        : muduo::net::TcpServer(loop, addr, name) {}

    // 公开 checkIdleConnection() 供测试调用
    using muduo::net::TcpServer::checkIdleConnection;
};

TEST(IdleConnectionTest, DetectIdleAndShutdown) {
    // 创建一个 EventLoop
    muduo::net::EventLoop loop;

    // 为测试准备本地和对端地址
    muduo::net::InetAddress localAddr("127.0.0.1", 12345);
    muduo::net::InetAddress peerAddr("127.0.0.1", 54321);

    // 创建一个 FakeTcpConnection（sockfd 设为 -1 仅作测试，不实际连接）
    auto conn = std::make_shared<FakeTcpConnection>(&loop, "test-conn", -1, localAddr, peerAddr);

    // 设置空闲超时时间为 1 秒
    std::chrono::seconds idleTimeout(1);

    // 将连接的 lastReceiveTime_ 设置为 2 秒前，模拟空闲状态
    conn->setLastReceiveTime(std::chrono::steady_clock::now() - std::chrono::seconds(2));

    // 创建一个测试用的 TcpServer 实例
    TestTcpServer server(&loop, localAddr, "TestServer");
    // 设置空闲超时参数（假设 TcpServer 有 setIdleTimeout 接口）
    server.setIdleTimeout(idleTimeout);

    // 调用检查空闲连接的方法
    server.checkIdleConnection(conn);

    // 预期：由于连接空闲超过1秒，checkIdleConnection 应该调用 shutdown()，从而设置 shutdownCalled_ 为 true
    EXPECT_TRUE(conn->shutdownCalled());
}
