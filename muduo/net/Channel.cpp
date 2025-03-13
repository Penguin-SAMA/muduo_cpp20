#include "Channel.h"

namespace muduo {
namespace net {

Channel::Channel(int fd)
    : fd_(fd)
    , events_(0)
    , revents_(0) {
}

Channel::~Channel() {
}

void Channel::handleEvent() {
    if ((revents_ & 0x20) && closeCallback_) {
        closeCallback_();
    } else if ((revents_ & 0x08) && errorCallback_) {
        errorCallback_();
    } else {
        if ((revents_ & 0x01) && readCallback_) {
            readCallback_();
        }
        if ((revents_ & 0x04) && writeCallback_) {
            writeCallback_();
        }
    }
}

void Channel::handleEventWithGuard() {
}

}
} // namespace muduo::net
