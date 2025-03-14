#if 0
#include "Channel.h"
#include "EventLoop.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo::net;

TEST(EventLoopPollerTest, BasicPollTest) {
    EventLoop loop;

    int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    ASSERT_GT(eventFd, 0);

    Channel channel(eventFd);
    channel.setReadCallback([&]() {
        std::cout << "Readable event triggered!" << std::endl;
        loop.quit();
    });
    channel.setEvents(EPOLLIN);

    loop.updateChannel(&channel);

    uint64_t value = 1;
    write(eventFd, &value, sizeof(value));

    loop.loop();

    close(eventFd);
}
#endif
