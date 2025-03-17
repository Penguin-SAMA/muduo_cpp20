#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <memory>

namespace muduo {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
    : baseLoop_(baseLoop)
    , started_(false)
    , numThreads_(numThreads)
    , next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    if (started_)
        return;
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        auto t = std::make_unique<EventLoopThread>(cb);
        EventLoop* loop = t->startLoop();
        threads_.push_back(std::move(t));
        loops_.push_back(loop);
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    if (loops_.empty()) {
        return baseLoop_;
    }

    EventLoop* loop = loops_[next_];
    next_ = (next_ + 1) % loops_.size();
    return loop;
}

}
} // namespace muduo::net
