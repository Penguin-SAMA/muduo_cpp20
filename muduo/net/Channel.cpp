#include "Channel.h"
#include "EventLoop.h"

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
    if ((revents_ & 0x20) && closeCallback_) {
        // 比如 EPOLLRDHUP 事件
        closeCallback_();
    } else if ((revents_ & 0x08) && errorCallback_) {
        // 比如 EPOLLERR 事件
        errorCallback_();
    } else {
        if ((revents_ & 0x01) && readCallback_) {
            // 比如 EPOLLIN 事件
            readCallback_();
        }
        if ((revents_ & 0x04) && writeCallback_) {
            // 比如 EPOLLOUT 事件
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
