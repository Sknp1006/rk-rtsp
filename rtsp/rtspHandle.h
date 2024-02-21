#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <atomic>

#include "PacketQueue.h"
#include "PacketIterator.h"
#include "VideoDecoder.h"
#include "VideoImageConverter.h"

using namespace av;

//获取当前时间(时间为毫秒)
inline int64_t getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return d.count();
}
//将获取到当前时间格式转换为00:00:00.000
const inline std::string millisecondToHHMMSSZZZ(int64_t &v)
{
    int hour = 0;
    int minute = 0;
    int second = 0;
    int millisecond = 0;

    second = v / 1000;
    millisecond = v % 1000;

    if (second > 60)
    {
        minute = second / 60;
        second = second % 60;
    }
    if (minute > 60)
    {
        hour = minute / 60;
        minute = minute % 60;
    }

    char buf[sizeof("00:00:00.000") + 1] = {
        0,
    };
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", hour, minute, second, millisecond);
    return buf;
}

inline std::string timestampToString(time_t timestamp) {
    char now[64];
    struct tm *ttime;
    ttime = localtime(&timestamp);
    strftime(now,64,"%Y-%m-%d %H:%M:%S",ttime);
    return std::string(now);
}

inline std::string formatTimestamp(const std::string& format)
{
    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);

    // 格式化时间戳为字符串
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), format.c_str());
    return ss.str();
}

struct RTSPInfo
{
    std::atomic<int64_t> sys_elapsedTime = 0;
    std::atomic<int64_t> sys_newElapsedTimeRefPoint = 0;
    std::atomic<int64_t> sys_baseElapsedTime = 0;

    std::atomic<AVRational> video_time_base;
    std::atomic<double> video_last_pkt_duration = 0;
    std::atomic<int64_t> video_last_pts = 0;
    std::atomic<int64_t> video_last_ptsTime = 0;
};

enum class RTSPState
{
    NOTCREATE,
    STOPPED,
    RUNNING,
    PAUSED,
};

class RTSPHandle
{
public:
    RTSPHandle(std::string rtsp_url);
    ~RTSPHandle();

    bool getFrame(cv::Mat &Frame);
    void stop();
    bool isRunning() { return (is_iterating_packet == RTSPState::RUNNING) || (is_decoding_rtsp == RTSPState::RUNNING); }
private:
    void iteratePacket();
    void decodeRtsp();

    bool quequeIsFull();
    void syncVideo(AVFrame *frame, bool &dropFrame, bool &dropQueque);
private:
    RTSPInfo rtspInfo;
    PacketQueue video_queque;
    std::shared_ptr<PacketIterator> packetIterator;
    std::shared_ptr<VideoDecoder> videoDecoder;
    std::shared_ptr<VideoImageConverter> videoImageConverter;

    // 拉流线程
    std::atomic<RTSPState> is_iterating_packet;
    std::shared_ptr<std::thread> iterating_packet_thread;
    std::atomic<bool> iterating_packet_thread_finished = false;
    std::atomic<bool> is_iterating_packet_thread_failed = false;

    // 解码线程
    std::atomic<RTSPState> is_decoding_rtsp;
    std::shared_ptr<std::thread> decoding_video_thread;
    std::atomic<bool> decoding_video_thread_finished = false;
    std::atomic<bool> is_decoding_video_thread_failed = false;

    std::atomic<bool> frame_ready = false;
    cv::Mat frame;
    std::mutex m_mutex;
};