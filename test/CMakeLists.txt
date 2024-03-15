Project(test LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/rtsp)
include_directories(${CMAKE_SOURCE_DIR}/utils)

#opencv4
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC rtsp utils)
target_link_libraries(${PROJECT_NAME} PUBLIC rtsp utils)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
