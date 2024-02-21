project(rtsp LANGUAGES CXX C)

#ffmpeg6.0
set(FFMPEG_DIR ${3rdparty_DIR}/ffmpeg)
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)
set(FFMPEG_LIBS libavcodec.a
                libavdevice.a
                libavfilter.a
                libavformat.a
                libavutil.a
                libswresample.a 
                libswscale.a)

#spdlog
set(SPDLOG_DIR ${3rdparty_DIR}/spdlog)
include_directories(${SPDLOG_DIR}/include)
link_directories(${SPDLOG_DIR}/lib)

#opencv4
find_package(OpenCV REQUIRED PATHS ${3rdparty_DIR}/opencv NO_DEFAULT_PATH)
# list(APPEND OPENCV_LIBS opencv_core opencv_imgproc opencv_imgcodecs)
list(APPEND OPENCV_LIBS opencv_world)

#SDL2
set(SDL2_DIR ${3rdparty_DIR}/SDL2/SDL2)
include_directories(${SDL2_DIR}/include)
link_directories(${SDL2_DIR}/lib)

#libmpp
set(LIBMPP_DIR ${3rdparty_DIR}/libmpp)
link_directories(${LIBMPP_DIR}/lib)
list(APPEND RKMPP_LIBS rockchip_mpp)

#libdrm
set(LIBDRM_DIR ${3rdparty_DIR}/libdrm)
link_directories(${LIBDRM_DIR}/lib)
list(APPEND LIBDRM_LIBS drm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")

add_library(rtsp STATIC ${SRC_FILES})
target_link_libraries(rtsp PRIVATE ${FFMPEG_LIBS} ${OPENCV_LIBS} spdlog SDL2 ${RKMPP_LIBS} ${LIBDRM_LIBS})

install(TARGETS rtsp DESTINATION lib)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${HEADERS} DESTINATION include/rtsp)
