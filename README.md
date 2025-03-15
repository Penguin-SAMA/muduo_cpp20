# muduo_cpp20

Muduo网络库作为一个基于C++11的现代化、事件驱动型的高性能网络库，以简洁的设计和高效的性能广泛受到开发者青睐。然而，随着C++语言标准的不断发展，尤其是C++20标准引入了众多新特性，如协程（coroutine）、格式化库（std::format）、更强大的并发工具等，这些新特性能够进一步优化和简化原有的网络库设计和实现。

本项目旨在利用C++20新特性对Muduo网络库进行全面的重构与升级，保留原有的经典设计思想，同时在代码质量、性能表现、易用性与可维护性上做进一步提升。此外，我们将引入更现代化的工具（例如spdlog替换原版日志系统），以及使用现代化的构建工具（xmake）和测试框架（GoogleTest）确保项目的现代化与跨平台兼容性。

## 创新点

**全面C++20支持**：如std::format、std::atomic、concepts等新特性。

**现代化日志系统**：使用spdlog替代原有日志系统。

**跨平台构建系统**：使用xmake构建。

**单元测试驱动开发**：GoogleTest编写单元测试。

## 项目简介

本项目将完整迁移Muduo网络库全部功能，包括基础设施（日志、时间戳）、网络核心（EventLoop、Poller、Channel）及连接管理（Acceptor、TcpServer、TcpConnection）。

- 开发环境：Windows 11 ArchWSL
- 构建工具：xmake
- 编译器：g++14.2 支持C++20
- 代码编辑器：LazyVim
- 调试工具：gdb
- 测试框架：GoogleTest
- 日志框架：spdlog
