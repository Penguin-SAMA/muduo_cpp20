#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

using namespace muduo::net;

TEST(AcceptorTest, BasicAccept) {
    muduo::base::initLogger("test_acceptor");

    EventLoop loop;
    InetAddress listenAddr(9877, false);
    Acceptor acceptor(&loop, listenAddr);

    acceptor.setNewConnectionCallback(
        [&](int sockfd, const InetAddress& peerAddr) {
            LOG_INFO("Got new connection from {}", peerAddr.toIpPort());
            ::close(sockfd);
            loop.quit();
        });

    acceptor.listen();

    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        system("echo 'hello' | nc 127.0.0.1 9877");
    });

    loop.loop();

    t.join();

    SUCCEED();
}
