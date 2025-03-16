#pragma once

#include <chrono>
#include <functional>

namespace muduo {
namespace net {

class Timer
{
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, std::chrono::steady_clock::time_point when, std::chrono::milliseconds interval);

    void run() const { callback_(); }

    std::chrono::steady_clock::time_point expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart();

private:
    const TimerCallback callback_;
    std::chrono::steady_clock::time_point expiration_;
    const std::chrono::milliseconds interval_;
    const bool repeat_;
};

}
} // namespace muduo::net
