#include "EventLoop.h"
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace muduo {
namespace net {

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false) {
}

EventLoop::~EventLoop() {
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    // 事件循环
    while (!quit_) {
        // 实际实现中会调用 I/O 多路复用来等待事件
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // 执行所有待执行任务
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

}
} // namespace muduo::net
