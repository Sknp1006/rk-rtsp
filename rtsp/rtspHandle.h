#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <atomic>

#include "PacketQueue.h"
#include "PacketIterator.h"
#include "VideoDecoder.h"
#include "VideoImageConverter.h"
#include "MatQueue.h"

using namespace av;

// 目前的帧率设计是只能慢不能快，以减少不必要的性能消耗
#ifndef MAXFRAMERATE
#define MAXFRAMERATE 100
#endif

struct RTSPInfo
{
    std::atomic<int64_t> sys_elapsedTime = 0;
    std::atomic<int64_t> sys_newElapsedTimeRefPoint = 0;
    std::atomic<int64_t> sys_baseElapsedTime = 0;
    std::atomic<int64_t> video_last_drop=0;

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
    PacketQueue audio_queque;
    std::shared_ptr<PacketIterator> packetIterator;
    std::shared_ptr<VideoDecoder> videoDecoder;
    std::shared_ptr<VideoImageConverter> videoImageConverter;
    // std::shared_ptr<MatQueue> matQueue;
    std::shared_ptr<QueueBase<cv::Mat>> matQueue;

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
};