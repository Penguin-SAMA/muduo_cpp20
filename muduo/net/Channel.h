#pragma once

#include <functional>
#include <memory>

namespace muduo {
namespace net {

class Channel : public std::enable_shared_from_this<Channel>
{
public:
    using EventCallback = std::function<void()>;

    Channel(int fd);
    ~Channel();

    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    void handleEvent();

    void setEvents(int events) { events_ = events; }
    int events() const { return events_; }

    void setRevents(int revents) { revents_ = revents; }
    int revents() const { return revents_; }

    int fd() const { return fd_; }

private:
    void handleEventWithGuard();

private:
    const int fd_;
    int events_;
    int revents_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}
} // namespace muduo::net
