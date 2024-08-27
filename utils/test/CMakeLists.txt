project(test_utils_time LANGUAGES CXX)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# test_utils_time
add_executable(${PROJECT_NAME} test_utils_time.cc)
target_link_libraries(test_utils_time PRIVATE utils GTest::GTest GTest::Main)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION test/bin
        )