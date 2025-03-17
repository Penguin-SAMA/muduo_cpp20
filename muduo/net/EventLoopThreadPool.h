#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace muduo {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback& cb = nullptr);

    // 如果启用了多线程模式，根据 Round-robin 获取下一个 EventLoop
    // 如果只有一个线程或者未启用多线程，就返回 baseLoop
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_; // 主 Loop
    bool started_;
    int numThreads_;
    int next_; // 轮询索引

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}
} // namespace muduo::net
