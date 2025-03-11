add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_toolchains("gcc")

target("muduo_core")
    set_kind("static")
    add_files("src/**/*.cpp")
    add_includedirs("include", "src")
    add_defines("_GLIBCXX_USE_NANOSLEEP")
    if is_plat("macosx") then
        add_frameworks("CoreFoundation")
    end
    add_syslinks("pthread")

