#include "InetAddress.h"
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

namespace muduo {
namespace net {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = ::htonl(ip);
    addr_.sin_port = ::htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        // 转换失败
        addr_.sin_addr.s_addr = ::htonl(INADDR_ANY);
    }
}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, (void*)&addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, (void*)&addr_.sin_addr, buf, sizeof(buf));
    int end = static_cast<int>(::strlen(buf));
    uint16_t port = ::ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

}
} // namespace muduo::net
