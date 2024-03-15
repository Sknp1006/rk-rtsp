cmake_minimum_required(VERSION 3.15)

project(rk LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# 打印C++版本
message(STATUS "C++ version: ${CMAKE_CXX_STANDARD}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -O0     #编译优化等级0 不优化
        -Wall   #开启所有警告
        -g      #包含调试信息
        -fPIC   #位置无关代码
    )
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
        -O3     #编译优化等级3
        -fPIC   #位置无关代码
    )
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()

add_subdirectory(rtsp)
add_subdirectory(utils)
add_subdirectory(test)

add_dependencies(test rtsp utils)
