#pragma once

#include "Poller.h"
#include <sys/epoll.h>
#include <vector>

namespace muduo {
namespace net {

class EpollPoller : public Poller
{
public:
    explicit EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    // 调用 epoll_wait
    void poll(int timeoutMs, ChannelList* activeChannels) override;
    // 更新 Channel 的关注事件
    void updateChannel(Channel* channel) override;
    // 移除 Channel 的关注事件
    void removeChannel(Channel* channel) override;

private:
    static constexpr int kInitEventListSize = 16; // epoll 事件列表的初始大小

    // 更新 epoll 事件
    void update(int operation, Channel* channel);
    // 填充活跃的 Channel
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    int epollfd_;                            // epoll 文件描述符
    std::vector<struct epoll_event> events_; // epoll 事件列表
};

}
} // namespace muduo::net
