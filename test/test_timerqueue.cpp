#include "EventLoop.h"
#include "Logging.h"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>

using namespace muduo::net;

TEST(TimerQueueTest, RunAfterTest) {
    EventLoop loop;
    std::atomic<bool> fired{false};

    loop.runAfter(std::chrono::milliseconds(100), [&]() {
        fired = true;
        loop.quit();
    });

    auto start = std::chrono::steady_clock::now();
    loop.loop();
    auto end = std::chrono::steady_clock::now();

    EXPECT_TRUE(fired);
    EXPECT_GE(end - start, std::chrono::milliseconds(100));
}

TEST(TimerQueueTest, RunEveryTest) {
    EventLoop loop;
    std::atomic<int> count{0};

    loop.runEvery(std::chrono::milliseconds(50), [&]() {
        count++;
        if (count >= 3) {
            loop.quit();
        }
    });

    auto start = std::chrono::steady_clock::now();
    loop.loop();
    auto end = std::chrono::steady_clock::now();

    EXPECT_EQ(count.load(), 3);
    EXPECT_GE(end - start, std::chrono::milliseconds(150));
}

TEST(TimerQueueTest, RunAtTest) {
    EventLoop loop;
    std::atomic<bool> fired{false};

    auto fireTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
    loop.runAt(fireTime, [&]() {
        fired = true;
        loop.quit();
    });

    auto start = std::chrono::steady_clock::now();
    loop.loop();
    auto end = std::chrono::steady_clock::now();

    EXPECT_TRUE(fired);
    EXPECT_GE(end - start, std::chrono::milliseconds(200));
}
