#include "Logging.h"
#include <gtest/gtest.h>

TEST(LoggingTest, BasicOutput) {
    // 初始化
    muduo::base::initLogger("test_looger");

    // 打印日志
    LOG_TRACE("This is a trace log");
    LOG_DEBUG("This is a debug log, val = {}", 123);
    LOG_INFO("This is a info log");
    LOG_WARN("This is a warn log");
    LOG_ERROR("This is a error log");
    LOG_CRITICAL("This is a critical log");

    SUCCEED();
}
