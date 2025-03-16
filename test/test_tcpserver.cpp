#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include "Socket.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace muduo::net;

TEST(TcpServerTest, EchoTest) {
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
    // 创建 Eventloop 和 TcpServer
    EventLoop loop;
    const uint16_t kPort = 12345;
    InetAddress listenAddr(kPort);
    TcpServer server(&loop, listenAddr, "EchoServer");

    // 设置服务器的回调
    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("New connection [{}] from {}",
                     conn->name(),
                     conn->peerAddress().toIpPort());
        }
    });

    server.setMessageCallback([](const TcpConnectionPtr& conn, const std::string& msg) {
        conn->send(msg);
    });

    // 启动服务器
    std::thread serverThread([&loop] {
        loop.loop();
    });
    server.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 获取服务器实际监听的端口
    uint16_t port = 0;
    port = 12345;

    int clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(clientfd > 0) << "Failed to create client socket";

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(kPort);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int ret = ::connect(clientfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        FAIL() << "connect failed: " << strerror(errno);
    }

    const char* testMsg = "Hello Muduo";
    ssize_t n = ::send(clientfd, testMsg, strlen(testMsg), 0);
    ASSERT_GT(n, 0) << "send failed: " << strerror(errno);

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    n = ::recv(clientfd, buf, sizeof(buf), 0);
    ASSERT_GT(n, 0) << "recv failed or connection closed: " << strerror(errno);

    std::string received(buf, n);
    EXPECT_EQ(received, testMsg);

    ::close(clientfd);

    loop.quit();
    serverThread.join();

    SUCCEED() << "TcpServer EchoTest passed!";
}
