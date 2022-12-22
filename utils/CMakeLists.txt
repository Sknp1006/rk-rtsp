project(utils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#spdlog
find_package(spdlog REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC utils_time.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# 直接编译到rtsp模块中
# install(TARGETS ${PROJECT_NAME} DESTINATION lib)
