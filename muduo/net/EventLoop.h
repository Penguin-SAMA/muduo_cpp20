#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace muduo {
namespace net {

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void runInLoop(Functor cb);

private:
    // 执行所有待执行任务
    void doPendingFunctors();

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;

    std::mutex mutex_;
    std::queue<Functor> pendingFunctors_;
};

}
} // namespace muduo::net
