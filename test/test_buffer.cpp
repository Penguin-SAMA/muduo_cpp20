#include "Buffer.h"
#include <gtest/gtest.h>
#include <string>

using namespace muduo::net;

TEST(BufferTest, BasicOperations) {
    Buffer buf;
    EXPECT_EQ(buf.readableBytes(), 0u);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize);

    std::string data = "Hello Muduo";
    buf.append(data.data(), data.size());
    EXPECT_EQ(buf.readableBytes(), data.size());
    EXPECT_GE(buf.writableBytes(), Buffer::kInitialSize - data.size());

    auto s = buf.retrieveAllAsString();
    EXPECT_EQ(s, data);
    EXPECT_EQ(buf.readableBytes(), 0u);
}

TEST(BufferTest, Prepend) {
    Buffer buf;
    std::string header = "HDR";
    buf.prepend(header.data(), header.size());
    EXPECT_EQ(buf.prependableBytes(), Buffer::kCheapPrepend - header.size());
}

TEST(BufferTest, EnsureWritable) {
    Buffer buf(8);
    std::string longStr(100, 'x');
    buf.append(longStr);
    EXPECT_GE(buf.readableBytes(), 100u);
}
