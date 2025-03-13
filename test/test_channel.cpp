#include "Channel.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(ChannelTest, BasicCallbackInvocation) {
    using namespace muduo::net;

    int fd = 1;

    Channel channel(fd);

    bool readCalled = false;
    bool writeCalled = false;
    bool closeCalled = false;
    bool errorCalled = false;

    channel.setReadCallback([&]() {
        readCalled = true;
        std::cout << "read callback invoked!\n";
    });
    channel.setWriteCallback([&]() {
        writeCalled = true;
        std::cout << "write callback invoked!\n";
    });
    channel.setCloseCallback([&]() {
        closeCalled = true;
        std::cout << "close callback invoked!\n";
    });
    channel.setErrorCallback([&]() {
        errorCalled = true;
        std::cout << "error callback invoked!\n";
    });

    channel.setRevents(0x01);
    channel.handleEvent();
    EXPECT_TRUE(readCalled);
    EXPECT_FALSE(writeCalled);
    EXPECT_FALSE(closeCalled);
    EXPECT_FALSE(errorCalled);

    readCalled = writeCalled = closeCalled = errorCalled = false;

    channel.setRevents(0x04);
    channel.handleEvent();
    EXPECT_FALSE(readCalled);
    EXPECT_TRUE(writeCalled);
    EXPECT_FALSE(closeCalled);
    EXPECT_FALSE(errorCalled);

    readCalled = writeCalled = closeCalled = errorCalled = false;

    channel.setRevents(0x20);
    channel.handleEvent();
    EXPECT_FALSE(readCalled);
    EXPECT_FALSE(writeCalled);
    EXPECT_TRUE(closeCalled);
    EXPECT_FALSE(errorCalled);

    readCalled = writeCalled = closeCalled = errorCalled = false;

    channel.setRevents(0x08);
    channel.handleEvent();
    EXPECT_FALSE(readCalled);
    EXPECT_FALSE(writeCalled);
    EXPECT_FALSE(closeCalled);
    EXPECT_TRUE(errorCalled);
}
