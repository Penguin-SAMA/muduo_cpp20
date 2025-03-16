#include "Buffer.h"
#include <cstddef>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace muduo {
namespace net {

const size_t Buffer::kInitialSize;
const size_t Buffer::kCheapPrepend;

ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];

    const size_t writable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

}
} // namespace muduo::net
