#pragma once
#include <string>
#include <vector>
#include <functional>
#include <opencv2/core.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace av
{
    using OnFrameConverted = std::function<void(void*,int)>;        // 传递AVPacket数据
    using OnFrameConverted2 = std::function<void(AVFrame*)>;        // 传递AVFrame数据
    using OnFrameConverted2Mat = std::function<void(cv::Mat&)>;     // 传递cv::Mat数据
    class VideoImageConverter
    {
    public:
        VideoImageConverter(const AVCodecContext *inputCodecCtx, std::string dstFormat);

        void convert(AVFrame *src, OnFrameConverted onFrameConverted);
        void FrameToMat(AVFrame *src, OnFrameConverted2 onFrameConverted2);   // 将yuv420p转成RGB32
        void FrameToMat(AVFrame *src, OnFrameConverted2Mat onFrameConverted2Mat);
        bool isOK() { return m_isOK; }

        ~VideoImageConverter();

    private:
        const AVCodecContext *m_inputCodecCtx = NULL;
        enum AVPixelFormat dst_pix_fmt;
        enum AVCodecID dst_codec_id;

        struct SwsContext *dst_sws_ctx = NULL;      // 这是给显示用的
        // struct SwsContext *sws_ctx = NULL;          // 这是给算法用的

        AVCodec *encoder_codec = NULL;
        AVCodecContext *encoder_codec_ctx = NULL;

        bool m_isOK = false;

    private:
        void scale(AVFrame *src, AVFrame *dst);
        void scale(AVFrame *src, AVFrame *dst, enum AVPixelFormat _dst_pix_fmt);
        void scale(AVFrame *src, AVFrame *dst, int dst_width, int dst_height, enum AVPixelFormat _dst_pix_fmt);
        void scale(AVFrame *src, cv::Mat &InOutMat, int dst_width, int dst_height, enum AVPixelFormat _dst_pix_fmt);
    };
}
