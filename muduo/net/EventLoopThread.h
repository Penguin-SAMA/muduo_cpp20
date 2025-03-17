#pragma once

#include "EventLoop.h"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace muduo {
namespace net {

class EventLoopThread
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = nullptr);
    ~EventLoopThread();

    // 启动线程并返回对应的 EventLoop
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

}
} // namespace muduo::net
