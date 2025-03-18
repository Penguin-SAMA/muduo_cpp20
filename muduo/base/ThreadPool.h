#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace muduo {
namespace base {

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 提交一个可调用对象到线程池，返回 std::future 以获取执行结果
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

private:
    // 工作线程的主函数
    void workerThread();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_{false};
};

template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using ReturnType = typename std::invoke_result<F, Args...>::type;

    auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<ReturnType> res = taskPtr->get_future();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool is stopped");
        }
        tasks_.emplace([taskPtr]() {
            (*taskPtr)();
        });
    }
    cond_.notify_one();
    return res;
}

}
} // namespace muduo::base
