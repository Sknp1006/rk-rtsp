Project(test LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/rtsp)
include_directories(${CMAKE_SOURCE_DIR}/utils)

# 第三方库
set(3rdparty ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

#opencv4
set(OpenCV_DIR ${3rdparty}/opencv/opencv-linux-aarch64/share/OpenCV)
find_package(OpenCV REQUIRED)

# rknn_api
set(RKNN_API_PATH ${CMAKE_CURRENT_SOURCE_DIR}/runtime/Linux/librknn_api)
set(RKNN_RT_LIB ${RKNN_API_PATH}/aarch64/librknnrt.so)
include_directories(${RKNN_API_PATH}/include)

# rga
set(RGA_PATH ${3rdparty}/rga)
set(RGA_LIB ${RGA_PATH}/libs/Linux/gcc-aarch64/librga.so)
include_directories(${RGA_PATH}/include)

add_executable(${PROJECT_NAME} main.cpp
                                preprocess.cc
                                postprocess.cc)
target_include_directories(${PROJECT_NAME} PUBLIC 
                                    rtsp 
                                    utils)
target_link_libraries(${PROJECT_NAME} PUBLIC rtsp utils 
                                        ${RKNN_RT_LIB}
                                        ${RGA_LIB}
                                        ${OpenCV_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/model DESTINATION bin)

install(FILES ${RKNN_RT_LIB} DESTINATION lib)
install(FILES ${RGA_LIB} DESTINATION lib)
