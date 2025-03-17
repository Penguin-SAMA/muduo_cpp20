#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace muduo {
namespace base {

// 初始化logger
void initLogger(const std::string& loggerName = "muduo_logger", bool async = false);

// 获取全局logger
std::shared_ptr<spdlog::logger> getLogger();

// 常用日志宏
#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(muduo::base::getLogger(), __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(muduo::base::getLogger(), __VA_ARGS__)
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(muduo::base::getLogger(), __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(muduo::base::getLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(muduo::base::getLogger(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(muduo::base::getLogger(), __VA_ARGS__)

}
} // namespace muduo::base
