#pragma once

#include "Channel.h"
#include "Timer.h"
#include <chrono>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

namespace muduo {
namespace net {

class EventLoop;

class TimerQueue
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    using TimerCallback = Timer::TimerCallback;

    void addTimer(TimerCallback cb, std::chrono::steady_clock::time_point when, std::chrono::milliseconds interval);

private:
    using TimerEntry = std::pair<std::chrono::steady_clock::time_point, Timer*>;
    using TimerList = std::set<TimerEntry>;
    using TimerHolder = std::unordered_set<std::unique_ptr<Timer>>;
    void handleRead();
    std::vector<TimerEntry> getExpired(std::chrono::steady_clock::time_point now);
    void reset(const std::vector<TimerEntry>& expired, std::chrono::steady_clock::time_point now);
    void resetTimerfd(std::chrono::steady_clock::time_point expiration);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
    TimerHolder timersHolder_;
};

}
} // namespace muduo::net
