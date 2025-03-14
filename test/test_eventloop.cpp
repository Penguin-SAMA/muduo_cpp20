#if 0
#include "EventLoop.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

TEST(EventLoopTest, RunAndQuit) {
    using namespace muduo::net;
    EventLoop loop;

    // 在另一个线程中启动事件循环
    std::thread loopThread([&loop]() {
        loop.loop();
    });

    // 在事件循环中加入一个任务，用于测试退出机制
    loop.runInLoop([&loop]() {
        // 延时后退出循环
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        loop.quit();
    });

    loopThread.join();

    SUCCEED();
}
#endif
