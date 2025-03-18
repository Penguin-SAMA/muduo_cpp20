#pragma once

#include "Channel.h"
#include "Poller.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <sys/eventfd.h>
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
    void assertInLoopThread();
    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void cancelTimer(TimerId timerId);

    TimerId runAt(std::chrono::steady_clock::time_point time, Timer::TimerCallback cb);
    TimerId runAfter(std::chrono::milliseconds delay, Timer::TimerCallback cb);
    TimerId runEvery(std::chrono::milliseconds interval, Timer::TimerCallback cb);

private:
    // 执行所有待执行任务
    void doPendingFunctors();

    void wakeup();
    void handleWakeup();

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;

    std::mutex mutex_;
    std::queue<Functor> pendingFunctors_;

    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;
    Channel wakeupChannel_;

    const std::thread::id threadId_;
};

}
} // namespace muduo::net
