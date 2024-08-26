#include "MatQueue.h"

namespace av
{
    MatQueue::MatQueue()
    {
    }

    MatQueue::~MatQueue()
    {
    }

    /// @brief 将一个元素放入队列
    /// @param mat
    /// @return
    int MatQueue::put(cv::Mat &mat)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (queue.size() >= max_size)
        {
            queue.pop(); // 移除队列头部的元素
        }
        queue.push(mat); // 将元素添加到队列尾部
        return 0;
    }

    /// @brief 从队列中取出一个元素
    /// @param mat
    /// @param block
    /// @return
    int MatQueue::get(cv::Mat &mat, bool block)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (block)
        {
            m_cv.wait(lock, [this]
                      { return !queue.empty(); });
        }
        if (queue.empty())
        {
            return -1;
        }
        mat = queue.front();
        queue.pop();
        return 0;
    }

    /// @brief 清空队列
    void MatQueue::flush()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!queue.empty())
        {
            queue.pop();
        }
    }

    /// @brief 退出队列
    void MatQueue::quit()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_quit = 1;
    }

    /// @brief 返回队列的最大长度
    /// @return
    int MatQueue::size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return max_size;
    }

    /// @brief 返回队列中的元素个数
    /// @return
    int MatQueue::numberOfMats()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return queue.size();
    }
} // namespace av