
#include "EventLoop.h"
#include "Logging.h"
#include "TcpConnection.h"
#include <gtest/gtest.h>

using namespace muduo::net;

TEST(TcpConnectionTest, SimpleEcho) {
    EventLoop loop;

    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

    InetAddress localAddr(8888);
    InetAddress peerAddr(9999);

    auto conn = std::make_shared<TcpConnection>(&loop, "test_conn", sv[0], localAddr, peerAddr);

    conn->setConnectionCallback([](const TcpConnection::TcpConnectionPtr& conn) {
        LOG_INFO("Connection established: {}", conn->name());
    });

    conn->setMessageCallback([](const TcpConnection::TcpConnectionPtr& conn, const std::string& msg) {
        LOG_INFO("Message received: {}", msg);
        conn->send(msg); // echo
    });

    conn->setCloseCallback([&loop](const TcpConnection::TcpConnectionPtr& conn) {
        LOG_INFO("Connection closed: {}", conn->name());
        loop.quit();
    });

    conn->connectEstablished();

    std::thread t([fd = sv[1]]() {
        const char* msg = "Hello, TcpConnection!";
        ::write(fd, msg, strlen(msg));
        char buf[1024] = {0};
        ssize_t n = ::read(fd, buf, sizeof(buf));
        LOG_INFO("Client received echo: {}", std::string(buf, n));
        ::close(fd);
    });

    loop.loop();
    t.join();

    SUCCEED();
}
