#pragma once
#include <string>
#include <functional>
#include <memory>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace av
{
    using OnFrameAvailableCallback = std::function<void(AVFrame *)>;
    class VideoDecoder
    {
    public:
        VideoDecoder(AVCodecContext *codecCtx);
        ~VideoDecoder();
        void decode(AVPacket *packet, OnFrameAvailableCallback callback);

        bool isOK() { return m_isOK; }

    private:
        AVCodecContext *codecCtx;
        AVFrame *pFrame = NULL;

        bool m_isOK = false;
    };
}