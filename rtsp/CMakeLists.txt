project(rtsp LANGUAGES CXX C)


find_package(PkgConfig REQUIRED)

#jellyfin-ffmpeg6.0
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec 
                                                    libavformat 
                                                    libavutil 
                                                    libswscale 
                                                    libavfilter 
                                                    libavdevice 
                                                    libswresample)

#spdlog
find_package(spdlog REQUIRED)

#opencv4
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
list(APPEND OPENCV_LIBS opencv_core opencv_imgproc opencv_imgcodecs)

#SDL2
find_package(SDL2 REQUIRED)

#libmpp
pkg_check_modules(LIBMPP REQUIRED IMPORTED_TARGET rockchip_mpp)

#libdrm
pkg_check_modules(LIBDRM REQUIRED IMPORTED_TARGET libdrm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")

message("BUILD: ${PROJECT_NAME}")
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC 
                                        PkgConfig::FFMPEG 
                                        spdlog::spdlog 
                                        ${OPENCV_LIBS} 
                                        PkgConfig::LIBMPP 
                                        PkgConfig::LIBDRM
                                        ${SDL2_LIBRARIES})

install(TARGETS rtsp DESTINATION lib)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/rkrtsp.h)
install(FILES ${HEADERS} DESTINATION include/rtsp)
