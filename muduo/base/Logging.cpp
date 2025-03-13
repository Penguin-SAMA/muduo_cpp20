// clang-format off
#include "Logging.h"
#include <cstdio>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog-inl.h>
#include <vector>
// clang-format on

namespace muduo {
namespace base {

// 在匿名空间维护一个全局指针
namespace {
std::shared_ptr<spdlog::logger> g_logger;
} // anonymous namespace

void initLogger(const std::string& loggerName) {
    try {
        // 这里使用两个sink，一个输出到控制台，一个写到滚动文件
        // 也可以只用其中一个

        // 1. 控制台 sink (带颜色)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l] %v");

        // 2. 滚动文件 sink， 最大 5MB， 保留 3 个文件
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("muduo.log", 1024 * 1024 * 5, 3);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][thread %t] %v");

        // 创建logger
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());

        // 设置日志级别
        logger->set_level(spdlog::level::trace);

        // 设置全局logger
        spdlog::register_logger(logger);
        g_logger = logger;

        // 输出一条启动信息
        SPDLOG_LOGGER_INFO(g_logger, "Logger initialized. Name: {}", loggerName);

    } catch (const spdlog::spdlog_ex& ex) {
        // 如果 spdlog 初始化失败，可以先用 fprintf 打印错误
        fprintf(stderr, "Log init failed: %s\n", ex.what());
    };
}

std::shared_ptr<spdlog::logger> getLogger() {
    if (!g_logger) {
        // 如果没有初始化，就默认初始化一个
        // 只输出到控制台
        initLogger("muduo_logger_default");
    }
    return g_logger;
}

}
} // namespace muduo::base
