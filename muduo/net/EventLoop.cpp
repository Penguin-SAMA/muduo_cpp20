#include "EventLoop.h"
#include "Channel.h"
#include "poller/EpollPoller.h"
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace muduo {
namespace net {

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , poller_(std::make_unique<EpollPoller>(this)) {
}

EventLoop::~EventLoop() {
}

void EventLoop::loop() {
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
}

void EventLoop::doPendingFunctors() {
    std::queue<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(functors, pendingFunctors_);
    }
    while (!functors.empty()) {
        functors.front()();
        functors.pop();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

}
} // namespace muduo::net
