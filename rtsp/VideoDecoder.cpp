#include "VideoDecoder.h"
#include <sstream>
#include <chrono>
#include <thread>
#include <iostream>

#include <spdlog/spdlog.h>
namespace av
{
    VideoDecoder::VideoDecoder(AVCodecContext *codecCtx)
        : codecCtx(codecCtx)
    {
        try
        {
            m_isOK = false;

            pFrame = av_frame_alloc();
            if (!pFrame)
            {
                std::stringstream msg;
                msg << "av_frame_alloc: Could not allocate frame";
                throw std::runtime_error(msg.str());
            }

            m_isOK = true;
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }
    VideoDecoder::~VideoDecoder()
    {
        SPDLOG_TRACE("VideoDecoder destruct");

        if (pFrame)
        {
            SPDLOG_TRACE("free pFrame");

            av_freep(&pFrame->data[0]);
            av_frame_free(&pFrame);

            pFrame = NULL;
        }
    }
    void VideoDecoder::decode(AVPacket *packet, OnFrameAvailableCallback callback)
    {
        try
        {
            // auto start = std::chrono::system_clock::now();
            int ret = avcodec_send_packet(codecCtx, packet);
            // SPDLOG_TRACE("avcodec_send_packet: '{0}'", ret);
            // auto end = std::chrono::system_clock::now();
            // std::chrono::duration<double, std::milli> elapsed = end - start;
            // SPDLOG_DEBUG("avcodec_send_packet: '{0}'ms", elapsed.count());      // 毫秒

            if (ret < 0)
            {
                std::stringstream msg;
                msg << "avcodec_send_packet: error";
                SPDLOG_ERROR("avcodec_send_packet: '{0}'", ret);
                throw std::runtime_error(msg.str());
            }
            while (ret >= 0)
            {
                std::shared_ptr<AVFrame> Clear(pFrame, [](AVFrame *p){ av_frame_unref(p); });
                ret = avcodec_receive_frame(codecCtx, pFrame);

                // // 打印帧类型
                //     // AV_PICTURE_TYPE_NONE = 0, ///< Undefined
                //     // AV_PICTURE_TYPE_I,     ///< Intra
                //     // AV_PICTURE_TYPE_P,     ///< Predicted
                //     // AV_PICTURE_TYPE_B,     ///< Bi-dir predicted
                //     // AV_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG-4
                //     // AV_PICTURE_TYPE_SI,    ///< Switching Intra
                //     // AV_PICTURE_TYPE_SP,    ///< Switching Predicted
                //     // AV_PICTURE_TYPE_BI,    ///< BI type
                // AVPictureType frameType = pFrame->pict_type;
                // char frameTypeName = av_get_picture_type_char(frameType);
                // std::cout << "frameType: " << frameTypeName << std::endl;

                SPDLOG_TRACE("avcodec_receive_frame: '{0}'", ret);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    break;
                }
                if (ret == 0)
                {
                    if (callback)
                        callback(pFrame);
                }
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }
}