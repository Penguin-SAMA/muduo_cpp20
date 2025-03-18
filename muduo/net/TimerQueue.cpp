#include "TimerQueue.h"
#include "EventLoop.h"
#include "Logging.h"
#include "TimerId.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iterator>
#include <memory>
#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo {
namespace net {

namespace {
int createTimerfd() {
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0) {
        LOG_ERROR("Failed in timerfd_create");
    }
    return fd;
}

void readTimerfd(int fd) {
    uint64_t expirations;
    ssize_t n = read(fd, &expirations, sizeof(expirations));
    if (n != sizeof(expirations)) {
        LOG_ERROR("TimerQueue::handleRead() reads {} bytes instead of 8", n);
    }
}
} // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop, timerfd_) {
    timerfdChannel_.setReadCallback([this] { handleRead(); });
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    loop_->removeChannel(&timerfdChannel_);
    close(timerfd_);
}

TimerId TimerQueue::addTimer(
    TimerCallback cb,
    std::chrono::steady_clock::time_point when,
    std::chrono::milliseconds interval) {
    auto timer = std::make_shared<Timer>(std::move(cb), when, interval);
    TimerId id(timer.get(), reinterpret_cast<int64_t>(timer.get()));

    loop_->runInLoop([this, timer]() mutable {
        bool earliestChanged = timers_.empty() || timer->expiration() < timers_.begin()->first;
        timers_.emplace(timer->expiration(), timer.get());
        timersHolder_.insert(timer);

        if (earliestChanged) {
            resetTimerfd(timers_.begin()->first);
        }
    });

    return id;
}

void TimerQueue::handleRead() {
    auto now = std::chrono::steady_clock::now();
    readTimerfd(timerfd_);
    auto expired = getExpired(now);

    for (const auto& timer : expired) {
        timer.second->run();
    }
    reset(expired, now);
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop([this, timerId] {
        auto it = std::find_if(timers_.begin(), timers_.end(),
                               [timerId](const auto& entry) {
                                   return entry.second == timerId.timer_;
                               });
        if (it != timers_.end()) {
            auto itHolder = std::find_if(timersHolder_.begin(), timersHolder_.end(),
                                         [timerId](const std::shared_ptr<Timer>& ptr) {
                                             return ptr.get() == timerId.timer_;
                                         });
            if (itHolder != timersHolder_.end()) {
                timersHolder_.erase(itHolder);
            }
            timers_.erase(it);
        }
    });
}

std::vector<TimerQueue::TimerEntry> TimerQueue::getExpired(std::chrono::steady_clock::time_point now) {
    std::vector<TimerEntry> expired;

    auto end = timers_.lower_bound({now, reinterpret_cast<Timer*>(UINTPTR_MAX)});
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    return expired;
}

void TimerQueue::reset(const std::vector<TimerEntry>& expired,
                       std::chrono::steady_clock::time_point now) {
    for (const auto& entry : expired) {
        Timer* timer = entry.second;
        if (timer->repeat()) {
            timer->restart();
            timers_.emplace(timer->expiration(), timer);
        } else {
            auto it = std::find_if(timersHolder_.begin(), timersHolder_.end(),
                                   [timer](const std::shared_ptr<Timer>& t) { return t.get() == timer; });
            if (it != timersHolder_.end()) {
                timersHolder_.erase(it);
            }
        }
    }

    if (!timers_.empty()) {
        resetTimerfd(timers_.begin()->first);
    }
}

void TimerQueue::resetTimerfd(std::chrono::steady_clock::time_point expiration) {
    itimerspec newValue{};
    auto duration = expiration - std::chrono::steady_clock::now();
    if (duration.count() < 0)
        duration = std::chrono::milliseconds(0);

    newValue.it_value.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    newValue.it_value.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
    timerfd_settime(timerfd_, 0, &newValue, nullptr);
}
}
} // namespace muduo::net
