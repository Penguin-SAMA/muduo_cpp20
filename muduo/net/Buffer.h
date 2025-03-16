
#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

namespace muduo {
namespace net {

/// @brief 实现一个可自动扩容的环形缓冲区，用于网络收发数据
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;   // 默认预留空间大小
    static const size_t kInitialSize = 1024; // 默认初始大小

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend) {
    }

    /// @brief 可读数据长度
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    /// @brief 可写空间长度
    size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    /// @brief 预留区长度
    size_t prependableBytes() const {
        return readerIndex_;
    }

    /// @brief 返回指向可读数据开始的指针
    const char* peek() const {
        return begin() + readerIndex_;
    }

    /// @brief 在可读区域后移除 len 字节
    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            // 清空
            retrieveAll();
        }
    }

    /// @brief 一次性移除全部可读数据
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    /// @brief 将可读区域转为 std::string
    std::string retrieveAllAsString() {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    /// @brief 写指针位置
    char* beginWrite() {
        return begin() + writerIndex_;
    }

    /// @brief 写指针位置 (只读)
    const char* beginWrite() const {
        return begin() + writerIndex_;
    }

    /// @brief 通知内部，已经写入了 len 字节
    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    /// @brief 追加数据
    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    /// @brief 追加数据（string_view 版本 - C++17/20）
    void append(std::string_view sv) {
        append(sv.data(), sv.size());
    }

    /// @brief 确保可写空间 >= len，如果不足则扩容
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    /// @brief 前置写入一段数据，比如在 buffer 前面插入包头
    void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    /// @brief 从 fd 上读取数据
    /// @return ssize_t : 读取的字节数
    ssize_t readFd(int fd, int* savedErrno);

private:
    char* begin() {
        return buffer_.data();
    }
    const char* begin() const {
        return buffer_.data();
    }

    /// @brief 当可写空间不足时扩容
    void makeSpace(size_t len) {
        // 检查可用空间（包括 prependable 的部分）是否足够
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            // 整个 buffer 都不够 => 直接扩容
            buffer_.resize(writerIndex_ + len);
        } else {
            // 够用，只是需要把可读数据往前挪
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

}
} // namespace muduo::net
  // namespace muduo::net
