#pragma once

#include <chrono>
#include <format>
#include <string>

namespace muduo {

class Timestamp
{
public:
    Timestamp() = default;

    static std::string nowAsString() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        return std::format("Current time: {}", std::ctime(&now_time_t));
    }
};

} // namespace muduo
