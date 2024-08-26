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
    template <class T>
    class QueueBase
    {
    public:
        QueueBase() = default;
        ~QueueBase() = default;

        int put(const T &data);
        int get(T &data, bool block);
        void flush();
        void quit();
        int size() const;

    private:
        int max_size = 1;
        std::queue<T> queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        int m_quit = 0;
    };

    template <class T>
    int QueueBase<T>::put(const T &item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (queue.size() >= max_size)
        {
            queue.pop();
        }
        queue.push(item);
        return 0;
    }

    template <class T>
    int QueueBase<T>::get(T &item, bool block)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (block)
        {
            m_cv.wait(lock, [this]
                      { return !queue.empty(); });
        }
        if (!queue.empty())
        {
            item = std::move(queue.front());
            queue.pop();
            return 0;
        }
        return -1;
    }

    template <class T>
    void QueueBase<T>::flush()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!queue.empty())
        {
            queue.pop();
        }
    }

    template <class T>
    void QueueBase<T>::quit()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_quit = 1;
        }
        m_cv.notify_all();
    }

    template <class T>
    int QueueBase<T>::size() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return queue.size();
    }

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