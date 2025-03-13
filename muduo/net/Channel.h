#pragma once

#include <functional>
#include <memory>

namespace muduo {
namespace net {

// Channel 负责封装一个文件描述符以及其关心的IO事件
// 并在事件就绪时调用对应的回调函数
// 关键点：
// Channel并不拥有这个文件描述符，也不会在析构时关闭这个文件描述符
// fd的生命周期由Acceptor/Connector/EventLoop等类管理
// Channel只负责监听fd上的IO事件，并在事件就绪时调用对应的回调函数
class Channel : public std::enable_shared_from_this<Channel>
{
public:
    using EventCallback = std::function<void()>;

    // @param fd 文件描述符
    Channel(int fd);
    ~Channel();

    // 设置可读事件回调函数
    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    // 设置可写事件回调函数
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    // 设置关闭事件回调函数
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    // 设置错误事件回调函数
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    // 当poller发现fd上的事件时，调用该函数
    void handleEvent();

    // 设置fd上关心的事件
    void setEvents(int events) { events_ = events; }
    int events() const { return events_; }

    // 设置fd上的事件
    void setRevents(int revents) { revents_ = revents; }
    int revents() const { return revents_; }

    // 返回fd
    int fd() const { return fd_; }

private:
    // 用于处理事件的函数，内部会调用handleEvent
    void handleEventWithGuard();

private:
    const int fd_; // 文件描述符
    int events_;   // 关心的事件
    int revents_;  // 就绪的事件

    // 四种事件的回调函数
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}
} // namespace muduo::net
