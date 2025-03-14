#pragma once

#include "Channel.h"
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

namespace muduo {
namespace net {

class EventLoop;

// @brief Poller是一个IO复用的抽象基类，由具体的epoll/poll实现
class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop* loop);
    virtual ~Poller() = default;

    // 轮询IO事件，等待 fd 上的事件发生
    virtual void poll(int timeoutMs, ChannelList* activeChannels) = 0;

    // 添加或者更新Channel
    virtual void updateChannel(Channel* channel) = 0;

    // 从Poller中移除Channel
    virtual void removeChannel(Channel* channel) = 0;

    // 判断Channel是否在Poller中
    virtual bool hasChannel(Channel* channel) const;

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;  // 存储所有活跃的Channel
    EventLoop* ownerLoop_; // Poller所属的EventLoop
};

}
} // namespace muduo::net
