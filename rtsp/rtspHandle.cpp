#include "rtspHandle.h"
#include <spdlog/spdlog.h>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
//获取当前时间(时间为毫秒)
int64_t getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return d.count();
}
/// @brief 构造函数
/// @param rtsp_url 
RTSPHandle::RTSPHandle(std::string rtsp_url)
{
    SPDLOG_TRACE("RTSPHandle::RTSPHandle()");
    is_iterating_packet.store(RTSPState::NOTCREATE);
    is_decoding_rtsp.store(RTSPState::NOTCREATE);
    try
    {
        this->packetIterator = std::make_shared<PacketIterator>();
        this->packetIterator->open(rtsp_url, true, true);          // 不成功会直接退出，不会开启线程
        if (this->packetIterator->videoIsOK())
        {
            matQueue = std::make_shared<MatQueue>();
            rtspInfo.video_time_base = this->packetIterator->time_base();
            AVCodecContext *codecContext = this->packetIterator->video_codecCtx();
            this->videoDecoder = std::make_shared<VideoDecoder>(codecContext);
            this->videoImageConverter = std::make_shared<VideoImageConverter>(codecContext, "bgr24");
        
            SPDLOG_TRACE("creating iterating_packet_thread.");
            this->iterating_packet_thread = std::make_shared<std::thread>(&RTSPHandle::iteratePacket, this);
            SPDLOG_TRACE("creating decoding_video_thread.");
            this->decoding_video_thread = std::make_shared<std::thread>(&RTSPHandle::decodeRtsp, this);
        }
        else
        {
            throw std::runtime_error("RTSPHandle::RTSPHandle() failed.");
        }
    }
    catch(const std::exception& e)
    {
        SPDLOG_ERROR("RTSPHandle::RTSPHandle() failed.");
        throw std::runtime_error("RTSPHandle::RTSPHandle() failed: " + std::string(e.what()));
    }
}
/// @brief 析构函数
RTSPHandle::~RTSPHandle()
{
    SPDLOG_TRACE("RTSPHandle::~RTSPHandle()");
}
/// @brief 获取帧
/// @param frame
/// @return 
bool RTSPHandle::getFrame(cv::Mat &Frame)
{
    SPDLOG_TRACE("RTSPHandle::getFrame()");
    if (matQueue->get(Frame, false) == 0)
    {
        SPDLOG_DEBUG("mat_queque.get: {0}", matQueue->size());
        return true;
    }
    return false;
}
/// @brief 停止拉流
void RTSPHandle::stop()
{
    SPDLOG_TRACE("RTSPHandle::stop()");
    // 必须先停止解码线程，再停止拉流线程

    // 停止解码线程
    if (this->is_decoding_rtsp.load() == RTSPState::RUNNING)
    {
        SPDLOG_TRACE("RTSPHandle::stop() stopping decoding_video_thread");
        this->is_decoding_rtsp.store(RTSPState::STOPPED);
        if (this->decoding_video_thread->joinable())
        {
            this->decoding_video_thread->join();
        }
        this->is_decoding_rtsp.store(RTSPState::NOTCREATE);
        SPDLOG_DEBUG("RTSPHandle::stop() stopped decoding_video_thread");
    }
    
    // 停止拉流线程
    if (this->is_iterating_packet.load() == RTSPState::RUNNING)
    {
        SPDLOG_TRACE("RTSPHandle::stop() stopping iterating_packet_thread");
        this->is_iterating_packet.store(RTSPState::STOPPED);
        if (this->iterating_packet_thread->joinable())
        {
            this->iterating_packet_thread->join();
        }
        this->is_iterating_packet.store(RTSPState::NOTCREATE);
        SPDLOG_DEBUG("RTSPHandle::stop() stopped iterating_packet_thread");
    }

    SPDLOG_TRACE("RTSPHandle::stop() finished.");
}
/// @brief 拉流线程
void RTSPHandle::iteratePacket()
{
    SPDLOG_TRACE("RTSPHandle::iteratePacket()");
    try
    {
        rtspInfo.sys_baseElapsedTime = 0;
        rtspInfo.sys_newElapsedTimeRefPoint = getCurrentTime(); // 获取当前时间

        AVPacket packet;
        av_init_packet(&packet);    // 初始化结构体
        enum AVMediaType mediaType; // 定义枚举类型

        bool withVideo = this->packetIterator->videoIsOK() && this->videoDecoder && this->videoDecoder->isOK();
        bool withAudio = false;

        this->is_iterating_packet.store(RTSPState::RUNNING);
        SPDLOG_DEBUG("withVideo: {0},withAudio: {1}", withVideo, withAudio);
        while(this->is_iterating_packet.load() == RTSPState::RUNNING)
        {
            if (this->quequeIsFull())
            {
                SPDLOG_DEBUG("queque is full, sleep 100ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (this->packetIterator->next(&packet, mediaType))
            {
                SPDLOG_DEBUG("packet: mediaType {0} size {1} pos {2} pts {3}", mediaType, packet.size, packet.pos, packet.pts);

                if (AVMEDIA_TYPE_VIDEO == mediaType && withVideo)
                {
                    this->video_queque.put(&packet);
                }
                else if (AVMEDIA_TYPE_AUDIO == mediaType && withAudio)
                {
                    this->audio_queque.put(&packet);
                }
            }
            av_packet_unref(&packet);
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 线程休眠1毫秒
        }
        this->iterating_packet_thread_finished = true;
    }
    catch (std::exception &ex)
    {
        SPDLOG_ERROR("RTSPHandle::iteratePacket() exception: '{0}'", ex.what());
        this->is_iterating_packet_thread_failed = true;
    }
}
/// @brief 解码线程
void RTSPHandle::decodeRtsp()
{
    try
    {
        SPDLOG_TRACE("RTSPHandle::decodeRtsp()");

        auto f_onConverted2Mat = [=](cv::Mat &mat)
        {
            SPDLOG_TRACE("decoding_video onConverted2Mat: rows {0} cols {1}", mat.rows, mat.cols);
            matQueue->put(mat);
        };

        // 处理解码后的帧数据
        auto f_onFrameAvailable = [=](AVFrame *frame)
        {
            SPDLOG_TRACE("decoding_video onFrameAvailable: pkt_dts {0} pts {1} pkt_duration {2}", frame->pkt_dts, frame->pts, frame->pkt_duration);

            // 定义两个变量用于控制帧的处理和队列的处理
            bool dropFrame = false;
            bool dropQueque = false;

            // 调用 syncVideo 函数，将解码后的帧和其他两个布尔变量作为参数传入，同步视频处理过程
            syncVideo(frame, dropFrame, dropQueque);
            // 如果为假
            if (!dropFrame) // 判断是否超时100毫秒，是则跳过不是则进行图像转换
            {
                this->videoImageConverter->FrameToMat(frame, f_onConverted2Mat);
            }
            if (dropQueque) // 判断是否超时1000毫秒，是则清空所有数据包
            {
                SPDLOG_TRACE("video_queque dropQueque");
                this->video_queque.flush(); // 清空视频队列中的所有数据包，用于处理丢弃的队列数据包
            }
        };

        this->is_decoding_rtsp.store(RTSPState::RUNNING);
        AVPacket packet;         // 定义一个结构体变量
        av_init_packet(&packet); // 初始化该结构体
        while(this->is_decoding_rtsp.load() == RTSPState::RUNNING)
        {
            int size = this->video_queque.numberOfPackets(); // 获取视频队列中的数据包数量
            int ret = this->video_queque.get(&packet, 1);    // 从消息队列中获取一个数据包
            SPDLOG_TRACE("video_queque.get: {0}, {1}", ret, size);
            if (ret > 0) // 获取数据包成功
            {
                this->videoDecoder->decode(&packet, f_onFrameAvailable); // 对获取到的数据包进行解码，然后将解码后的帧传给f_onFrameAvailable
            }
            av_packet_unref(&packet); // 释放packet
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 线程休眠1毫秒
        }
        this->decoding_video_thread_finished = true; // 将线程设置为true，表示线程执行完毕
    }
    catch (std::exception &ex)
    {
        SPDLOG_ERROR("RTSPHandle::decodeRtsp() exception: '{0}'", ex.what());
        this->is_decoding_video_thread_failed = true;
    }
}
/// @brief 判断队列是否已满
/// @return 
bool RTSPHandle::quequeIsFull()
{
    int total = 60; // 设置队列最大数

    if (this->video_queque.numberOfPackets() > total)
    {
        SPDLOG_DEBUG("queque is full,video_queque: {0}", this->video_queque.numberOfPackets());
        return true;
    }
    return false;
}
/// @brief 同步视频
/// @param frame 
/// @param dropFrame 
/// @param dropQueque 
void RTSPHandle::syncVideo(AVFrame *frame, bool &dropFrame, bool &dropQueque)
{
    // 如果frame的包持续时间存在，则将其转换为毫秒并保存到last_pkt_duration变量中
    if (frame->pkt_duration)
    {
        rtspInfo.video_last_pkt_duration = frame->pkt_duration * 1000 * av_q2d(rtspInfo.video_time_base);
    }

    bool pts_NOPTS = false;
    if (frame->pkt_dts != AV_NOPTS_VALUE)
    {
        rtspInfo.video_last_pts = frame->pkt_dts;
        SPDLOG_TRACE("frame->pkt_dts: {}", frame->pkt_dts);
    }
    else if (frame->pts != AV_NOPTS_VALUE)
    {
        rtspInfo.video_last_pts = frame->pts;
        SPDLOG_TRACE("frame->pts: {}", frame->pts);
    }
    else // 如果frame的包持续时间存在，则将其转换为毫秒并保存到last_pkt_duration变量中
    {
        SPDLOG_WARN("NOPTS frame");

        pts_NOPTS = true;
    }

    if (!pts_NOPTS)
    {
        // 如果last_pts_NOPTS不为true，则计算last_ptsTime（最后pts的时间戳）
        rtspInfo.video_last_ptsTime = (int64_t)((rtspInfo.video_last_pts * 1000 * av_q2d(rtspInfo.video_time_base)));
        SPDLOG_TRACE("video_last_ptsTime: {}", rtspInfo.video_last_ptsTime);

        // 基于系统时钟计算从开始播放流逝的时间
        rtspInfo.sys_elapsedTime = rtspInfo.sys_baseElapsedTime + (getCurrentTime() - rtspInfo.sys_newElapsedTimeRefPoint);
        // 向系统时钟同步，如果last_ptsTime比m_elapsedTime大（提前），则延迟一段时间
        if (rtspInfo.video_last_ptsTime > rtspInfo.sys_elapsedTime) // early
        {
            auto diff = static_cast<unsigned int>(rtspInfo.video_last_ptsTime - rtspInfo.sys_elapsedTime);

            SPDLOG_TRACE("video,early: {}", diff);
            // av_usleep(diff * 1000);     // 使帧延迟
        }
        else if (rtspInfo.video_last_ptsTime < rtspInfo.sys_elapsedTime) // 如果小于m_elapsedTime（落后），则记录落后时间
        {
            auto diff = static_cast<unsigned int>(rtspInfo.sys_elapsedTime - rtspInfo.video_last_ptsTime);

            SPDLOG_DEBUG("video,later: {}", diff);
            // 检查是否大于100毫秒，若大于则记录警告并置dropFrame为true，重置baseElapsedTime和newElapsedTimeRefPoint
            if (diff > 100)
            {
                SPDLOG_WARN("video,drop frame and reset baseElapsedTime");

                dropFrame = true;

                // rtspInfo.sys_baseElapsedTime = rtspInfo.video_last_ptsTime;
                rtspInfo.sys_baseElapsedTime = (rtspInfo.video_last_ptsTime.load());     // 原子变量之间的赋值操作
                rtspInfo.sys_newElapsedTimeRefPoint = getCurrentTime();
            }
            if (diff > 1000)
            {
                SPDLOG_WARN("video,drop dropQueque");

                dropQueque = true;
            }
        }
    }
    {
        if (rtspInfo.video_last_drop == 0)
        {
            rtspInfo.video_last_drop = (rtspInfo.video_last_ptsTime.load());
            return;
        }

        auto diff_of_drop = static_cast<unsigned int>(rtspInfo.video_last_ptsTime - rtspInfo.video_last_drop);
        if (diff_of_drop >= (1000.0 / MAXFRAMERATE))
        {
            rtspInfo.video_last_drop = (rtspInfo.video_last_ptsTime.load());
        }
        else
        {
            dropFrame = true;
        }
    }
}
