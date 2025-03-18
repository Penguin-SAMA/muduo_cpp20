#include "ThreadPool.h"
#include <future>
#include <iterator>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <stdexcept>
#include <type_traits>

namespace muduo {
namespace base {

ThreadPool::ThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        workers_.emplace_back([this] {
            workerThread();
        });
    }
}

ThreadPool::~ThreadPool() {
    // 通知所有线程停止
    stop_ = true;
    cond_.notify_all();

    // 等待所有工作线程退出
    for (auto& t : workers_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPool::workerThread() {
    while (!stop_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

}
} // namespace muduo::base
