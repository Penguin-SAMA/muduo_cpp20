
#include "Logging.h"
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

// 测试写入日志的数量（适当提高）
const int kLogCount = 1000000; // 100万条
const int kThreadCount = 4;    // 4个线程同时写日志
const int kMessageSize = 1024; // 每条消息大小1KB

void writeLogs(int count, int threadId) {
    std::string largeMessage(kMessageSize, 'X');
    for (int i = 0; i < count; ++i) {
        LOG_INFO("Thread {} Logging test message {} - {}", threadId, i, largeMessage);
    }
}

long long performLoggingTest(bool async) {
    using namespace muduo::base;
    initLogger(async ? "async_logger" : "sync_logger", async);

    auto start = std::chrono::high_resolution_clock::now();

    // 使用多线程并发写日志
    std::vector<std::future<void>> futures;
    for (int i = 0; i < kThreadCount; ++i) {
        futures.push_back(std::async(std::launch::async, writeLogs, kLogCount / kThreadCount, i));
    }
    for (auto& fut : futures) {
        fut.get();
    }

    auto end = std::chrono::high_resolution_clock::now();
    spdlog::shutdown();

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

TEST(LoggingTest, AsyncShouldBeFasterInMultiThreaded) {
    auto syncDuration = performLoggingTest(false);
    std::cout << "[MultiThreaded Sync Logging] " << syncDuration << " ms" << std::endl;

    auto asyncDuration = performLoggingTest(true);
    std::cout << "[MultiThreaded Async Logging] " << asyncDuration << " ms" << std::endl;

    // 一般异步模式在这种场景下显著更快
    EXPECT_LT(asyncDuration, syncDuration);
}
