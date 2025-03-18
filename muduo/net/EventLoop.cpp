#include "EventLoop.h"
#include "Channel.h"
#include "Logging.h"
#include "poller/EpollPoller.h"
#include <chrono>
#include <exception>
#include <mutex>
#include <queue>
#include <sys/eventfd.h>
#include <thread>
#include <vector>

namespace muduo {
namespace net {

namespace {
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_ERROR("Failed in eventfd creation");
    }
    return evtfd;
}
} // namespace

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , poller_(std::make_unique<EpollPoller>(this))
    , timerQueue_(std::make_unique<TimerQueue>(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(this, wakeupFd_)
    , threadId_(std::this_thread::get_id()) {
    wakeupChannel_.setReadCallback([this] { handleWakeup(); });
    wakeupChannel_.enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_.disableAll();
    removeChannel(&wakeupChannel_);
    close(wakeupFd_);
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes {} bytes instead of 8", n);
    }
}

void EventLoop::handleWakeup() {
    uint64_t one;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleWakeup() reads {} bytes instead of 8", n);
    }
}

void EventLoop::loop() {
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    // 事件循环
    while (!quit_) {
        std::vector<Channel*> activeChannels;
        poller_->poll(10000, &activeChannels);

        for (auto channel : activeChannels) {
            channel->handleEvent();
        }

        doPendingFunctors();
    }

    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::runInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push(std::move(cb));
    }
    wakeup();
}

void EventLoop::assertInLoopThread() {
    if (!isInLoopThread()) {
        std::terminate();
    }
}

void EventLoop::cancelTimer(TimerId timerId) {
    timerQueue_->cancel(timerId);
}

void EventLoop::doPendingFunctors() {
    std::queue<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(functors, pendingFunctors_);
    }
    while (!functors.empty()) {
        try {
            functors.front()();
        } catch (const std::exception& ex) {
            LOG_ERROR("Exception in functor: {}", ex.what());
        } catch (...) {
            LOG_ERROR("Unknown exception in functor");
        }
        functors.pop();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

TimerId EventLoop::runAt(std::chrono::steady_clock::time_point time, Timer::TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time, std::chrono::milliseconds(0));
}

TimerId EventLoop::runAfter(std::chrono::milliseconds delay, Timer::TimerCallback cb) {
    auto time = std::chrono::steady_clock::now() + delay;
    return timerQueue_->addTimer(std::move(cb), time, std::chrono::milliseconds(0));
}

TimerId EventLoop::runEvery(std::chrono::milliseconds interval, Timer::TimerCallback cb) {
    auto time = std::chrono::steady_clock::now() + interval;
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

}
} // namespace muduo::net
