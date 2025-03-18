
// test_muduo_all.cpp

#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <future>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

// 包含 Muduo 模块的头文件
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include "Socket.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "ThreadPool.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Timestamp.h"

using namespace std::chrono_literals;
using namespace muduo;
using namespace muduo::base;
using namespace muduo::net;

//
// 1. Logging 测试
//
TEST(MuduoTest, Logging) {
    initLogger("test_logger", false);
    auto logger = getLogger();
    ASSERT_NE(logger, nullptr);
    LOG_INFO("Test logging message: {}", "Hello, Muduo");
}

//
// 2. Timestamp 测试
//
TEST(MuduoTest, Timestamp) {
    std::string ts = Timestamp::nowAsString();
    EXPECT_NE(ts.find("Current time:"), std::string::npos);
}

//
// 3. InetAddress 测试
//
TEST(MuduoTest, InetAddress) {
    InetAddress addr("127.0.0.1", 8080);
    EXPECT_EQ(addr.toIp(), "127.0.0.1");
    std::string ipPort = addr.toIpPort();
    EXPECT_NE(ipPort.find("8080"), std::string::npos);
}

//
// 4. Socket 测试
//
TEST(MuduoTest, Socket) {
    int sockfd = createNonblocking();
    EXPECT_GE(sockfd, 0);
    Socket s(sockfd);
    s.setNonBlockAndCloseOnExec();
    s.shutdownWrite();
}

//
// 5. ThreadPool 测试
//
TEST(MuduoTest, ThreadPool) {
    ThreadPool pool(4);
    auto futureResult = pool.submit([](int a, int b) {
        std::this_thread::sleep_for(50ms);
        return a + b;
    },
                                    10, 20);
    int result = futureResult.get();
    EXPECT_EQ(result, 30);
}

//
// 6. EventLoop 定时器测试：runAfter
//
TEST(MuduoTest, EventLoopTimer) {
    EventLoop loop;
    std::atomic<bool> timerFired{false};
    loop.runAfter(100ms, [&]() {
        timerFired = true;
        loop.quit();
    });
    loop.loop();
    EXPECT_TRUE(timerFired);
}

//
// 7. TimerQueue Cancel 测试
//
TEST(MuduoTest, TimerQueueCancel) {
    EventLoop loop;
    std::atomic<bool> fired{false};
    TimerId id = loop.runAfter(100ms, [&]() {
        fired = true;
    });
    loop.cancelTimer(id);
    loop.runAfter(200ms, [&]() { loop.quit(); });
    loop.loop();
    EXPECT_FALSE(fired);
}

//
// 8. TcpConnection 测试（使用 socketpair 模拟双向通信）
//
TEST(MuduoTest, TcpConnectionSocketPair) {
    int sv[2];
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    ASSERT_EQ(ret, 0);

    EventLoop loop;
    InetAddress localAddr; // 空地址
    InetAddress peerAddr;  // 空地址
    auto conn = std::make_shared<TcpConnection>(&loop, "TestConn", sv[0], localAddr, peerAddr);
    std::atomic<bool> messageReceived{false};
    conn->setMessageCallback([&](const TcpConnection::TcpConnectionPtr&, const std::string& msg) {
        messageReceived = true;
    });
    // 为确保 EventLoop 内的 pendingFunctors 被执行，使用 runAfter 定时退出
    loop.runInLoop([&] { conn->connectEstablished(); });
    loop.runAfter(150ms, [&] { loop.quit(); });

    // 向对端写入数据
    const char* msg = "test message";
    ::write(sv[1], msg, strlen(msg));
    loop.loop();

    EXPECT_TRUE(messageReceived);
    conn->shutdown();
    close(sv[1]);
}

//
// 9. TcpServer Echo 测试（在主线程运行 EventLoop）
//
TEST(MuduoTest, TcpServerEcho) {
    // 在主线程中创建 EventLoop 和 TcpServer
    EventLoop loop;
    InetAddress listenAddr(12345);
    TcpServer server(&loop, listenAddr, "TestEchoServer");
    std::atomic<bool> connectionUp{false};
    std::atomic<bool> messageEchoed{false};

    server.setConnectionCallback([&](const TcpConnection::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionUp = true;
        }
    });
    server.setMessageCallback([&](const TcpConnection::TcpConnectionPtr& conn, const std::string& msg) {
        conn->send(msg);
        messageEchoed = true;
    });
    server.setThreadNum(1);
    server.start();

    // 启动客户端操作在独立线程
    std::thread clientThread([]() {
        std::this_thread::sleep_for(200ms);
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_GE(clientfd, 0);
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
        int retConnect = connect(clientfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
        ASSERT_EQ(retConnect, 0);
        std::this_thread::sleep_for(200ms);
        std::string testMsg = "Hello, Muduo";
        ssize_t n = send(clientfd, testMsg.data(), testMsg.size(), 0);
        ASSERT_GT(n, 0);
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        n = recv(clientfd, buf, sizeof(buf) - 1, 0);
        EXPECT_GT(n, 0);
        std::string received(buf, n);
        EXPECT_EQ(received, testMsg);
        close(clientfd);
    });

    // 让服务器运行一段时间，处理客户端请求
    loop.runAfter(1s, [&]() { loop.quit(); });
    loop.loop();
    clientThread.join();
}

//
// 10. ThreadPool Heavy Task 测试
//
TEST(MuduoTest, ThreadPoolHeavyTask) {
    ThreadPool pool(4);
    std::atomic<int> sum{0};
    int numTasks = 100;
    auto task = [&sum]() {
        for (int i = 0; i < 1000; ++i) {
            sum.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::future<void>> futures;
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(pool.submit(task));
    }
    for (auto& fut : futures) {
        fut.get();
    }
    EXPECT_EQ(sum.load(), numTasks * 1000);
}
