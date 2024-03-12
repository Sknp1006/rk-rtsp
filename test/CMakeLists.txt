Project(test LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/rtsp)

#opencv4
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC rtsp)
target_link_libraries(${PROJECT_NAME} PUBLIC rtsp)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
