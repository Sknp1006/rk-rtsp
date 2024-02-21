Project(test LANGUAGES CXX)

include_directories(${3rdparty_DIR}/opencv/include/opencv4)
link_directories(${3rdparty_DIR}/opencv/lib)

include_directories(${3rdparty_DIR}/ffmpeg/include)
link_directories(${3rdparty_DIR}/ffmpeg/lib)

include_directories(${3rdparty_DIR}/SDL2/SDL2/include)
link_directories(${3rdparty_DIR}/SDL2/SDL2/lib)

link_directories(${3rdparty_DIR}/spdlog/lib)

#libmpp
set(LIBMPP_DIR ${3rdparty_DIR}/libmpp)
link_directories(${LIBMPP_DIR}/lib)

#libdrm
set(LIBDRM_DIR ${3rdparty_DIR}/libdrm)
link_directories(${LIBDRM_DIR}/lib)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC rtsp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
