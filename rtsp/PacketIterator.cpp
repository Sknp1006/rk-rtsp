#include "PacketIterator.h"
#include <sstream>
#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>
namespace av
{
    PacketIterator::PacketIterator()
    {
    }
    PacketIterator::~PacketIterator()
    {
        SPDLOG_TRACE("PacketIterator destruct");
        if (m_formatCtx)
        {
            SPDLOG_TRACE("avformat_close_input");
            avformat_close_input(&m_formatCtx);
        }
        if (m_video_codecCtx)
        {
            SPDLOG_TRACE("avcodec_free_context m_video_codecCtx");
            avcodec_free_context(&m_video_codecCtx);
            m_video_codecCtx = NULL;
        }
        if (m_audio_codecCtx)
        {
            SPDLOG_TRACE("avcodec_free_context m_audio_codecCtx");
            avcodec_free_context(&m_audio_codecCtx);
            m_audio_codecCtx = NULL;
        }
    }
    void PacketIterator::open(std::string url, bool video, bool audio)
    {
        m_url = url;

        m_formatCtx = avformat_alloc_context();
        AVDictionary *options = NULL;
        av_dict_set(&options, "max_delay", "5000000", 0);  // 设置最大延时 5s
        av_dict_set(&options, "rtsp_transport", "tcp", 0); // 设置连接为TCP
        av_dict_set(&options, "stimeout", "20000000", 0);  // 设置20s超时断开连接时间
        av_dict_set(&options, "buffer_size", "1024000", 0);
        if (avformat_open_input(&m_formatCtx, m_url.c_str(), NULL, &options) != 0)
        {
            std::stringstream msg;
            msg << "avformat_open_input: can't open " << m_url;
            SPDLOG_ERROR(msg.str().c_str());
            throw std::runtime_error(msg.str());
        }
        if (avformat_find_stream_info(m_formatCtx, NULL) < 0)
        {
            std::stringstream msg;
            msg << "avformat_find_stream_info: Could't find stream infomation," << m_url;
            SPDLOG_ERROR(msg.str().c_str());
            throw std::runtime_error(msg.str());
        }

        if (video)
        {
            m_videoIsOK = false;
            try
            {
                SPDLOG_TRACE("PacketIterator::open() init video");

                m_video_stream = av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
                if (m_video_stream < 0)
                {
                    std::stringstream msg;
                    msg << "av_find_best_stream: Didn't find a stream" << m_url;
                    SPDLOG_ERROR(msg.str().c_str());
                    throw std::runtime_error(msg.str());
                }

                m_video_codecParameters = m_formatCtx->streams[m_video_stream]->codecpar;
                m_video_codec = avcodec_find_decoder(m_video_codecParameters->codec_id);
                SPDLOG_DEBUG("m_video_codecParameters->codec_id: {0}", m_video_codecParameters->codec_id);
                SPDLOG_DEBUG("m_video_codec: {0}", m_video_codec->id);
                if (m_video_codec == NULL)
                {
                    std::stringstream msg;
                    msg << "avcodec_find_decoder: codec not found,codec_id: " << m_video_codecParameters->codec_id;
                    SPDLOG_ERROR(msg.str().c_str());
                    throw std::runtime_error(msg.str());
                }

                m_video_codecCtx = avcodec_alloc_context3(m_video_codec);
                if (avcodec_parameters_to_context(m_video_codecCtx, m_video_codecParameters) != 0)
                {
                    std::stringstream msg;
                    msg << "avcodec_parameters_to_context: Couldn't copy codec context";
                    SPDLOG_ERROR(msg.str().c_str());
                    throw std::runtime_error(msg.str());
                }

                if (avcodec_open2(m_video_codecCtx, m_video_codec, NULL) < 0)
                {
                    std::stringstream msg;
                    msg << "avcodec_open2: Could not open codec";
                    SPDLOG_ERROR(msg.str().c_str());
                    throw std::runtime_error(msg.str());
                }

                m_time_base = m_formatCtx->streams[m_video_stream]->time_base;
                m_videoIsOK = true;
            }
            catch (std::exception &ex)
            {
                SPDLOG_ERROR("exception: '{0}'", ex.what());
            }
        }
        if (audio)
        {
            m_audioIsOK = false;
            try
            {
                SPDLOG_TRACE("-----init audio-----");

                m_audio_stream = av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
                if (m_audio_stream < 0)
                {
                    std::stringstream msg;
                    msg << "av_find_best_stream: Didn't find a stream" << m_url;
                    throw std::runtime_error(msg.str());
                }

                m_audio_codecParameters = m_formatCtx->streams[m_audio_stream]->codecpar;
                m_audio_codec = avcodec_find_decoder(m_audio_codecParameters->codec_id);
                if (m_audio_codec == NULL)
                {
                    std::stringstream msg;
                    msg << "avcodec_find_decoder: codec not found,codec_id: " << m_audio_codecParameters->codec_id;
                    throw std::runtime_error(msg.str());
                }

                m_audio_codecCtx = avcodec_alloc_context3(m_audio_codec);
                if (avcodec_parameters_to_context(m_audio_codecCtx, m_audio_codecParameters) != 0)
                {
                    std::stringstream msg;
                    msg << "avcodec_parameters_to_context: Couldn't copy codec context";
                    throw std::runtime_error(msg.str());
                }

                if (avcodec_open2(m_audio_codecCtx, m_audio_codec, NULL) < 0)
                {
                    std::stringstream msg;
                    msg << "avcodec_open2: Could not open codec";
                    throw std::runtime_error(msg.str());
                }

                m_audioIsOK = true;
            }
            catch (std::exception &ex)
            {
                SPDLOG_ERROR("exception: '{0}'", ex.what());
            }
        }
    }
    bool PacketIterator::next(AVPacket *packet, enum AVMediaType &type)
    {
        int ret = av_read_frame(m_formatCtx, packet);
        if (ret < 0)
        {
            if (m_formatCtx->pb->error == 0) /* no error; wait for user input */
            {
                SPDLOG_WARN("no error; wait for user input");
            }
        }
        else if (ret == 0)
        {
            if (packet->stream_index == m_video_stream && m_videoIsOK)
            {
                type = AVMEDIA_TYPE_VIDEO;
                return true;
            }
            else if (packet->stream_index == m_audio_stream && m_audioIsOK)
            {
                type = AVMEDIA_TYPE_AUDIO;
                return true;
            }
        }

        return false;
    }
}