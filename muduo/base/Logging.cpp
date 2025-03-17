// clang-format off
#include "Logging.h"
#include <cstdio>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <vector>
// clang-format on

namespace muduo {
namespace base {

// 在匿名空间维护一个全局指针
namespace {
std::shared_ptr<spdlog::logger> g_logger;
} // anonymous namespace

void initLogger(const std::string& loggerName, bool async) {
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
        std::vector<spdlog::sink_ptr> sinks{file_sink};

        if (async) {
            // 初始化异步日志线程池
            // queue_size = 8192, worker_count = 1
            spdlog::init_thread_pool(65536, 2);

            // 创建异步logger
            auto logger = std::make_shared<spdlog::async_logger>(
                loggerName,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::overrun_oldest);

            logger->set_level(spdlog::level::trace);
            spdlog::register_logger(logger);
            g_logger = logger;
        } else {
            // 同步logger
            auto logger = std::make_shared<spdlog::logger>(
                loggerName,
                sinks.begin(),
                sinks.end());
            logger->set_level(spdlog::level::trace);
            spdlog::register_logger(logger);
            g_logger = logger;
        }

        SPDLOG_LOGGER_INFO(g_logger, "Logger initialized. Name: {}, async={}", loggerName, async);

    } catch (const spdlog::spdlog_ex& ex) {
        // 如果 spdlog 初始化失败，可以先用 fprintf 打印错误
        fprintf(stderr, "Log init failed: %s\n", ex.what());
    };
}

std::shared_ptr<spdlog::logger> getLogger() {
    if (!g_logger) {
        // 如果没有初始化，就默认初始化一个
        // 只输出到控制台
        initLogger("muduo_logger_default", false);
    }
    return g_logger;
}

}
} // namespace muduo::base
