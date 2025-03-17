set_project("muduo_cpp20")

set_languages("c++20")
set_toolchains("gcc")

add_rules("mode.debug", "mode.release")

add_requires("spdlog")

target("muduo_cpp20")
    set_kind("static")
    add_includedirs("muduo/base", "muduo/net")
    add_files("muduo/base/*.cpp", "muduo/net/*.cpp")
    add_files("muduo/net/poller/*.cpp")
    add_packages("spdlog")

target("muduo_tests")
    set_kind("binary")
    add_files("test/test_timerqueue.cpp")
    add_deps("muduo_cpp20")
    add_links("pthread", "gtest", "gtest_main")
    add_includedirs("muduo/base", "muduo/net")
    add_packages("spdlog")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
    if is_mode("debug") then
        add_defines("SPDLOG_DEBUG")
    end

target("muduo")
    set_kind("binary")
    add_files("main.cpp")
    add_deps("muduo_cpp20")
    add_links("pthread", "gtest", "gtest_main")
    add_includedirs("muduo/base", "muduo/net")
    add_packages("spdlog")
