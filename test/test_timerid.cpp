
#include "EventLoop.h"
#include "TimerId.h"
#include <atomic>
#include <gtest/gtest.h>

using namespace muduo::net;

TEST(TimerIdTest, CancelTimer) {
    EventLoop loop;
    std::atomic<bool> fired{false};

    TimerId id = loop.runAfter(std::chrono::milliseconds(100), [&]() {
        fired = true;
    });

    loop.cancelTimer(id); // 取消定时器

    loop.runAfter(std::chrono::milliseconds(200), [&]() { loop.quit(); });
    loop.loop();

    EXPECT_FALSE(fired); // 应该不会触发
}
