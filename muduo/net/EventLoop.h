#pragma once

#include "Channel.h"
#include "Poller.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

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
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    // 执行所有待执行任务
    void doPendingFunctors();

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;

    std::mutex mutex_;
    std::queue<Functor> pendingFunctors_;

    std::unique_ptr<Poller> poller_;
};

}
} // namespace muduo::net
