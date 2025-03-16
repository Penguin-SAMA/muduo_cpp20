#include "Timer.h"
#include <chrono>

namespace muduo {
namespace net {
Timer::Timer(TimerCallback cb, std::chrono::steady_clock::time_point when, std::chrono::milliseconds interval)
    : callback_(std::move(cb))
    , expiration_(when)
    , interval_(interval)
    , repeat_(interval.count() > 0) {
}

void Timer::restart() {
    if (repeat_) {
        expiration_ = std::chrono::steady_clock::now() + interval_;
    } else {
        expiration_ = std::chrono::steady_clock::time_point::min();
    }
}

}
} // namespace muduo::net
