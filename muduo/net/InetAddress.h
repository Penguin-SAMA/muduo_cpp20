#pragma once

#include <cstring>
#include <netinet/in.h>
#include <string>

namespace muduo {
namespace net {

// @brief 封装 IP + Port
class InetAddress
{
public:
    // 构造一个IPv4地址，指定端口，默认:0.0.0.0
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    // 根据字符串 IP 构造，例如"127.0.0.1", "192.168.1.100"
    InetAddress(const std::string& ip, uint16_t port);

    // 直接用现有的 sockaddr_in 构造
    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_(addr) {}

    std::string toIp() const;
    std::string toIpPort() const;

    const struct sockaddr_in& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

private:
    struct sockaddr_in addr_;
};

}
} // namespace muduo::net
