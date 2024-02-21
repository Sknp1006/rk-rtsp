#pragma once
#include <string>
#include <functional>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace av
{
    class PacketIterator
    {
    public:
        PacketIterator();
        ~PacketIterator();
        void open(std::string url, bool video, bool audio);
        bool next(AVPacket *packet, enum AVMediaType &type);

        AVFormatContext *formatCtx() { return m_formatCtx; }

        bool videoIsOK() { return m_videoIsOK; }
        int video_Stream() { return m_video_stream; }
        AVCodecParameters *video_codecParameters() { return m_video_codecParameters; }
        AVCodec *video_codec() { return (AVCodec *)m_video_codec; }
        AVCodecContext *video_codecCtx() { return m_video_codecCtx; }

        bool audioIsOK() { return m_audioIsOK; }
        int audio_Stream() { return m_audio_stream; }
        AVCodecParameters *audio_codecParameters() { return m_audio_codecParameters; }
        AVCodec *audio_codec() { return (AVCodec *)m_audio_codec; }
        AVCodecContext *audio_codecCtx() { return m_audio_codecCtx; }

        AVRational time_base() { return m_time_base; }
    private:
        std::string m_url;
        AVFormatContext *m_formatCtx = NULL;

        bool m_videoIsOK = false;
        int m_video_stream = -1;
        AVCodecParameters *m_video_codecParameters = NULL;
        const AVCodec *m_video_codec = NULL;
        AVCodecContext *m_video_codecCtx = NULL;

        bool m_audioIsOK = false;
        int m_audio_stream = -1;
        AVCodecParameters *m_audio_codecParameters = NULL;
        const AVCodec *m_audio_codec = NULL;
        AVCodecContext *m_audio_codecCtx = NULL;

        AVRational m_time_base;
    };
}