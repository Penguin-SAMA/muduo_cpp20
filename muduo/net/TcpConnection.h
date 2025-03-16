#pragma once

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include <functional>
#include <memory>
#include <string>

namespace muduo {
namespace net {

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const std::string&)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void send(const std::string& message);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();

    bool connected() const { return state_ == kConnected; }

    const InetAddress& peerAddress() const { return peerAddr_; }
    const InetAddress& localAddress() const { return localAddr_; }

private:
    void
    handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    enum State {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected
    };
    void setState(State s) { state_ = s; }

private:
    EventLoop* loop_;
    const std::string name_;
    State state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;

    std::string inputBuffer_;
    std::string outputBuffer_;
};

}
} // namespace muduo::net
