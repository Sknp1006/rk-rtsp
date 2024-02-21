cmake_minimum_required(VERSION 3.15)

project(rk LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# 打印C++版本
message(STATUS "C++ version: ${CMAKE_CXX_STANDARD}")

# 第三方库
set(3rdparty_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

add_compile_options(
    -O0      #编译优化等级0 不优化
    # -Wall   #开启所有警告
    # -g      #包含调试信息
    -fPIC   #位置无关代码
)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)

add_subdirectory(rtsp)
add_subdirectory(test)
