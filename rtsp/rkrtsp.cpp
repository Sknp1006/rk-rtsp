#include <spdlog/spdlog.h>
#include "rkrtsp.h"
#include "rtspHandle.h"
#include <string>

/// @brief 创建RTSP对象
/// @param rtsp_url 
/// @return 
LONG64 CreateRKRTSP(const char* rtsp_url)
{
    RTSPHandle* rtsp = new RTSPHandle(rtsp_url);
    return (LONG64)rtsp;
}
/// @brief 销毁RTSP对象
/// @param ctx 
void ReleaseRKRTSP(LONG64 ctx)
{
    RTSPHandle* ptr = (RTSPHandle*)ctx;
    free(ptr);
    delete ptr;
}
/// @brief 获取RTSP拉流解码后的帧
/// @param ctx 对象指针
/// @param buffer 图像数据
/// @param width 图像宽度
/// @param height 图像高度
/// @param channels 图像通道数
/// @param type 图像类型
/// @return 
API_EXPORT int GetFrame(LONG64 ctx, ImageData &frame)
{
    RTSPHandle* rtsp = (RTSPHandle*)ctx;
    cv::Mat image;
    if (rtsp->getFrame(image))
    {
        frame.width = image.cols;
        frame.height = image.rows;
        frame.channels = image.channels();
        frame.type = image.type();
        frame.data = new unsigned char[image.total() * image.elemSize()];
        memcpy(frame.data, image.data, image.total() * image.elemSize());
        return 0;
    }
    return -1;
}
/// @brief 停止RTSP拉流解码
/// @param ctx 
void Stop(LONG64 ctx)
{
    RTSPHandle* rtsp = (RTSPHandle*)ctx;
    rtsp->stop();
}
/// @brief 检查RTSP是否正在运行
/// @param ctx 
/// @return 
bool IsRunning(LONG64 ctx)
{
    RTSPHandle* rtsp = (RTSPHandle*)ctx;
    return rtsp->isRunning();
}