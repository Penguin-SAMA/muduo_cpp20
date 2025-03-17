
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include <string>

using namespace muduo;
using namespace muduo::net;

void onMessage(const TcpConnection::TcpConnectionPtr& conn, const std::string& msg) {
    conn->send(msg);
}

int main() {
    EventLoop loop;
    InetAddress listenAddr(12345);
    TcpServer server(&loop, listenAddr, "MultithreadedEcho");

    server.setConnectionCallback([](const TcpConnection::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("Connection UP: {}", conn->name());
        } else {
            LOG_INFO("Connection DOWN: {}", conn->name());
        }
    });

    server.setMessageCallback(onMessage);
    server.setThreadNum(4); // 4个IO线程

    server.start();
    loop.loop();
    return 0;
}
