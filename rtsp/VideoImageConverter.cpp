#include "VideoImageConverter.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>
#include <opencv2/core.hpp>
namespace av
{

    VideoImageConverter::VideoImageConverter(const AVCodecContext *inputCodecCtx, std::string dstFormat)
        : m_inputCodecCtx(inputCodecCtx)
    {
        try
        {
            m_isOK = false;

            if (strstr(dstFormat.c_str(), "jpg"))
            {
                this->dst_pix_fmt = AV_PIX_FMT_YUVJ420P;
                this->dst_codec_id = AV_CODEC_ID_MJPEG;
            }
            else if (strstr(dstFormat.c_str(), "png"))
            {
                this->dst_pix_fmt = AV_PIX_FMT_RGB24;
                this->dst_codec_id = AV_CODEC_ID_PNG;
            }
            else if (strstr(dstFormat.c_str(), "bmp"))
            {
                this->dst_pix_fmt = AV_PIX_FMT_RGB32;
                this->dst_codec_id = AV_CODEC_ID_BMP;
            }
            else if (strstr(dstFormat.c_str(), "bgr24"))
            {
                this->dst_pix_fmt = AV_PIX_FMT_BGR24;
                this->dst_codec_id = AV_CODEC_ID_BMP;
            }
            else
            {
                std::stringstream msg;
                msg << "unsupported format: " << dstFormat;
                SPDLOG_ERROR(msg.str().c_str());
                throw std::runtime_error(msg.str());
            }

            SPDLOG_DEBUG("inputCodecCtx->width = '{0}'", inputCodecCtx->width);
            SPDLOG_DEBUG("inputCodecCtx->height = '{0}'", inputCodecCtx->height);
            SPDLOG_DEBUG("pix_fmt: '{0}'", m_inputCodecCtx->pix_fmt);
            SPDLOG_DEBUG("dst_pix_fmt: '{0}'", dst_pix_fmt);
            dst_sws_ctx = sws_getContext(
                m_inputCodecCtx->width, m_inputCodecCtx->height, m_inputCodecCtx->pix_fmt, m_inputCodecCtx->width, m_inputCodecCtx->height,
                this->dst_pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
            if (!dst_sws_ctx)
            {
                std::stringstream msg;
                msg << "sws_getContext: error";
                SPDLOG_ERROR(msg.str().c_str());
                throw std::runtime_error(msg.str());
            }

            encoder_codec = (AVCodec *)avcodec_find_encoder(dst_codec_id);
            if (!encoder_codec)
            {
                std::stringstream msg;
                msg << "avcodec_find_encoder: error";
                SPDLOG_ERROR(msg.str().c_str());
                throw std::runtime_error(msg.str());
            }

            encoder_codec_ctx = avcodec_alloc_context3(encoder_codec);
            if (!encoder_codec_ctx)
            {
                std::stringstream msg;
                msg << "avcodec_alloc_context3: error";
                SPDLOG_ERROR(msg.str().c_str());
                throw std::runtime_error(msg.str());
            }
            encoder_codec_ctx->bit_rate = 400000;
            encoder_codec_ctx->width = m_inputCodecCtx->width;
            encoder_codec_ctx->height = m_inputCodecCtx->height;
            encoder_codec_ctx->time_base = AVRational{1, 25};
            encoder_codec_ctx->pix_fmt = dst_pix_fmt;
            if (dst_codec_id == AV_CODEC_ID_MJPEG)
            {
                // 9 JPEG quality
                encoder_codec_ctx->flags |= AV_CODEC_FLAG_QSCALE;
                encoder_codec_ctx->global_quality = FF_QP2LAMBDA * 9;
            }

            int ret = avcodec_open2(encoder_codec_ctx, encoder_codec, NULL);
            SPDLOG_TRACE("avcodec_open2: '{0}'", ret);

            if (ret < 0)
            {
                std::stringstream msg;
                msg << "avcodec_open2: error";
                SPDLOG_ERROR(msg.str().c_str());
                throw std::runtime_error(msg.str());
            }

            m_isOK = true;
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }

    VideoImageConverter::~VideoImageConverter()
    {
        SPDLOG_TRACE("VideoImageConverter destruct");
        if (dst_sws_ctx)
        {
            SPDLOG_TRACE("sws_freeContext dst_sws_ctx");
            sws_freeContext(dst_sws_ctx);
            dst_sws_ctx = NULL;
        }
        if (encoder_codec_ctx)
        {
            SPDLOG_TRACE("avcodec_free_context encoder_codec_ctx");
            avcodec_free_context(&encoder_codec_ctx);
            encoder_codec_ctx = NULL;
        }
    }

    void VideoImageConverter::convert(AVFrame *src, OnFrameConverted onFrameConverted)
    {
        try
        {
            AVFrame *dst = av_frame_alloc();
            if (!dst)
            {
                std::stringstream msg;
                msg << "av_frame_alloc: error";
                throw std::runtime_error(msg.str());
            }
            std::shared_ptr<AVFrame> dst_Clear(dst, [](AVFrame *p)
                                               {
                                            av_freep(&p->data[0]);
                                            av_frame_free(&p); });

            // scale to dst format
            scale(src, dst);

            int ret = avcodec_send_frame(encoder_codec_ctx, dst);
            SPDLOG_TRACE("encoder avcodec_send_frame: '{0}'", ret);
            if (ret < 0)
            {
                std::stringstream msg;
                msg << "encoder avcodec_send_frame: error";
                throw std::runtime_error(msg.str());
            }

            AVPacket packet;
            av_init_packet(&packet);
            while (ret >= 0)
            {
                std::shared_ptr<AVPacket> Clear(&packet, [](AVPacket *p)
                                                { av_packet_unref(p); });

                ret = avcodec_receive_packet(encoder_codec_ctx, &packet);

                SPDLOG_TRACE("encoder avcodec_receive_packet: '{0}'", ret);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                if (ret < 0)
                {
                    break;
                }

                if (onFrameConverted)
                {
                    onFrameConverted(packet.data, packet.size);
                }
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }

    /// @brief 将AVFrame转成void *buffer, int size
    /// @param src
    /// @param onFrameConverted
    void VideoImageConverter::FrameToMat(AVFrame *src, OnFrameConverted2 onFrameConverted2)
    {
        try
        {
            SPDLOG_TRACE("VideoImageConverter::FrameToMat---------------");
            AVFrame *dst = av_frame_alloc();
            if (!dst)
            {
                std::stringstream msg;
                msg << "av_frame_alloc: error";
                throw std::runtime_error(msg.str());
            }
            std::shared_ptr<AVFrame> dst_Clear(dst, [](AVFrame *p){
                                av_freep(&p->data[0]);
                                av_frame_free(&p); });
            // scalexs(src, dst, AV_PIX_FMT_BGR24);

            scale(src, dst, 640, 640, AV_PIX_FMT_BGR24);

            if (onFrameConverted2)
            {
                onFrameConverted2(dst);
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }

    void VideoImageConverter::FrameToMat(AVFrame *src, OnFrameConverted2Mat onFrameConverted2Mat)
    {
        try
        {
            SPDLOG_TRACE("VideoImageConverter::FrameToMat()");

            cv::Mat InOutMat;
            scale(src, InOutMat, src->width, src->height, AV_PIX_FMT_BGR24);        // 这里的FMT无效

            if (onFrameConverted2Mat)
            {
                onFrameConverted2Mat(InOutMat);
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("exception: '{0}'", ex.what());
        }
    }

    void VideoImageConverter::scale(AVFrame *src, AVFrame *dst)
    {
        dst->format = dst_pix_fmt;
        dst->width = m_inputCodecCtx->width;
        dst->height = m_inputCodecCtx->height;
        dst->pts = 0;

        int ret = av_image_alloc(
            dst->data, dst->linesize, dst->width, dst->height,
            (enum AVPixelFormat)dst->format, 32);

        SPDLOG_TRACE("av_image_alloc: '{0}'", ret);

        if (ret < 0)
        {
            std::stringstream msg;
            msg << "av_image_alloc: error";
            throw std::runtime_error(msg.str());
        }

        ret = sws_scale(dst_sws_ctx, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
        SPDLOG_TRACE("sws_scale: '{0}'", ret);

        if (ret < 0)
        {
            std::stringstream msg;
            msg << "sws_scale: error";
            throw std::runtime_error(msg.str());
        }
    }

    void VideoImageConverter::scale(AVFrame *src, AVFrame *dst, enum AVPixelFormat _dst_pix_fmt)
    {
        try
        {

            dst->format = _dst_pix_fmt;
            dst->width = src->width;
            dst->height = src->height;
            dst->pts = 0;

            int ret = av_image_alloc(
                dst->data, dst->linesize, dst->width, dst->height,
                (enum AVPixelFormat)dst->format, 32);

            if (ret < 0)
            {
                std::stringstream msg;
                msg << "av_image_alloc: error";
                SPDLOG_ERROR(msg.str());
                throw std::runtime_error(msg.str());
            }
            ret = sws_scale(dst_sws_ctx, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);

            if (ret < 0)
            {
                std::stringstream msg;
                msg << "sws_scale: error";
                SPDLOG_ERROR(msg.str());
                throw std::runtime_error(msg.str());
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("scalexs exception: '{0}'", ex.what());
        }
    }

    void VideoImageConverter::scale(AVFrame *src, AVFrame *dst, int dst_width, int dst_height, enum AVPixelFormat _dst_pix_fmt)
    {
        try
        {
            dst->format = _dst_pix_fmt;
            dst->width = dst_width;
            dst->height = dst_height;
            dst->pts = 0;

            int ret = av_image_alloc(
                dst->data, dst->linesize, dst->width, dst->height,
                (enum AVPixelFormat)dst->format, 32);

            if (ret < 0)
            {
                std::stringstream msg;
                msg << "av_image_alloc: error";
                SPDLOG_ERROR(msg.str());
                throw std::runtime_error(msg.str());
            }

            ret = sws_scale(dst_sws_ctx, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
            if (ret < 0)
            {
                std::stringstream msg;
                msg << "sws_scale: error";
                SPDLOG_ERROR(msg.str());
                throw std::runtime_error(msg.str());
            }
        }
        catch (std::exception &ex)
        {
            SPDLOG_ERROR("scale exception: '{0}'", ex.what());
        }
    }

    void VideoImageConverter::scale(AVFrame *src, cv::Mat &InOutMat, int dst_width, int dst_height, enum AVPixelFormat _dst_pix_fmt)
    {
        try
        {
            InOutMat = cv::Mat(dst_height, dst_width, CV_8UC3);

            int cvLinesizes[1];  
            cvLinesizes[0] = InOutMat.step1();  

            sws_scale(dst_sws_ctx, src->data, src->linesize, 0, dst_height, &InOutMat.data, cvLinesizes);  
            SPDLOG_TRACE("cv::Mat size: ({0},{1})", InOutMat.rows, InOutMat.cols);

            if (InOutMat.empty())
            {
                std::stringstream msg;
                msg << "scale to mat: error";
                SPDLOG_ERROR(msg.str());
                throw std::runtime_error(msg.str());
            }
        }
        catch(const std::exception& ex)
        {
            SPDLOG_ERROR("scale exception: '{0}'", ex.what());
        }
    }
}

