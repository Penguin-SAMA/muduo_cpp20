#include "Timestamp.h"
#include <gtest/gtest.h>

TEST(TimestampTest, NowAsStringTest) {
    std::string timeStr = muduo::Timestamp::nowAsString();

    ASSERT_FALSE(timeStr.empty()) << "Time string is empty!";
}
