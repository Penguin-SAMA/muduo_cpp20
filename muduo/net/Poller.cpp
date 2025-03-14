#include "Poller.h"
#include "EventLoop.h"
#include <cassert>

namespace muduo {
namespace net {

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop) {}

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

}
} // namespace muduo::net
