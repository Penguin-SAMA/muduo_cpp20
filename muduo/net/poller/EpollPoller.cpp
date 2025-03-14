#include "EpollPoller.h"
#include "Logging.h"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

namespace muduo {
namespace net {

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create(EPOLL_CLOEXEC)) // 创建 epoll 文件描述符
    , events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_ERROR("EpollPoller: epoll_create1 failed: {}", strerror(errno));
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

void EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, events_.data(), events_.size(), timeoutMs);
    if (numEvents > 0) {
        LOG_INFO("{} events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE("epoll_wait timeout");
    } else {
        LOG_ERROR("epoll_wait error: {}", strerror(errno));
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    const int index = channel->events();
    if (channels_.find(channel->fd()) == channels_.end()) {
        channels_[channel->fd()] = channel;
        update(EPOLL_CTL_ADD, channel);
    } else {
        update(EPOLL_CTL_MOD, channel);
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    channels_.erase(channel->fd());
    update(EPOLL_CTL_DEL, channel);
}

void EpollPoller::update(int operation, Channel* channel) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = channel->events();
    ev.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, channel->fd(), &ev) < 0) {
        LOG_ERROR("epoll_ctl error: {}", strerror(errno));
    }
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

}
} // namespace muduo::net
