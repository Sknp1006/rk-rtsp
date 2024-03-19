#pragma once
#include <opencv2/core.hpp>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

/*
这个文件是一个队列的实现，用于存储Opencv的图像帧
*/

namespace av
{
    class MatQueue
    {
    public:
        MatQueue();
        ~MatQueue();

        int put(cv::Mat &mat);
        int get(cv::Mat &mat, bool block);
        void flush();
        void quit();
        int size();
        int numberOfMats();

    private:
        int max_size = 1;
        std::queue<cv::Mat> queue;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        int m_quit = 0;
    };
};