project(utils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#spdlog
find_package(spdlog REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${PROJECT_NAME} SHARED utils_time.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)