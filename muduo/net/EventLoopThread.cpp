#include "EventLoopThread.h"
#include "EventLoop.h"
#include <mutex>

namespace muduo {
namespace net {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : loop_(nullptr)
    , exiting_(false)
    , thread_()
    , callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

EventLoop* EventLoopThread::startLoop() {
    // 启动新线程
    thread_ = std::thread([this] { threadFunc(); });

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    // 在线程中创建 EventLoop
    EventLoop localLoop;
    if (callback_) {
        callback_(&localLoop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &localLoop;
        cond_.notify_one();
    }

    localLoop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

}
} // namespace muduo::net
