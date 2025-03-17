#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

namespace muduo {
namespace net {

// 只对 fd 做简单的引用，不负责关闭 fd
// 所以构造函数不负责打开 fd，析构函数不负责关闭 fd
Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0) {
}

Channel::~Channel() {
}

void Channel::handleEvent() {
    // 为了安全，先做一个shared_ptr的延长生命周期
    // 防止在回调函数中将自己析构
    handleEventWithGuard();
}

void Channel::handleEventWithGuard() {
    // 根据revents_ 的值，调用对应的回调函数
    // 判断条件根据 poll 的返回值定义
    if ((revents_ & EPOLLHUP) || (revents_ & EPOLLRDHUP) || (revents_ & EPOLLERR)) {
        // 比如 EPOLLRDHUP 事件
        if (closeCallback_)
            closeCallback_();
    } else {
        if (revents_ & EPOLLIN) {
            // 比如 EPOLLIN 事件
            if (readCallback_)
                readCallback_();
        }
        if (revents_ & EPOLLOUT) {
            // 比如 EPOLLOUT 事件
            if (writeCallback_)
                writeCallback_();
        }
    }
}

void Channel::disableAll() {
    events_ = 0;
    update();
}

void Channel::update() {
    loop_->updateChannel(this);
}

}
} // namespace muduo::net
