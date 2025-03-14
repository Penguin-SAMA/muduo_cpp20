#include "EventLoop.h"
#include "poller/EpollPoller.h"
#include <gtest/gtest.h>

using namespace muduo::net;

TEST(EpollPollerTest, BasicPoll) {
    EventLoop loop;
    EpollPoller poller(&loop);

    SUCCEED();
}
