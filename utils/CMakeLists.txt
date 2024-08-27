project(utils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#spdlog
find_package(spdlog REQUIRED)

# utils.a
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} STATIC utils_time.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

add_subdirectory(test)
add_dependencies(test_utils_time ${PROJECT_NAME})
