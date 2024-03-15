#pragma once

#include <chrono>
#include <string>
#include <iomanip>
#include <atomic>
#include <thread>
#include <functional>

namespace Utils
{
    std::string getCurrentDate();
    std::string getCurrentTime();
    std::string timestamp2string(time_t timestamp, const char* fmt);
    time_t string2timestamp(const char* str, const char* fmt);

    // using timenow = std::bind(std::chrono::system_clock::now);
    using timestamp = std::chrono::time_point<std::chrono::system_clock>;

    enum class TimerState
    {
        WAITING = -1,
        RUNNING = 0,
        STOPPED = 1,
        QUIT = 2
    };
    using OnCallback = std::function<void(TimerState)>;
    class Timer
    {
    public:
        Timer();                                                            // 委托构造
        Timer(const char* Begin, const char* End);                          // 不需要完整的时间格式
        ~Timer();

        void start();
        void stop();
        TimerState getState() const;
        std::string getBeginTime();
        std::string getEndTime();
    protected:
        void formatTime();
        bool updateDate();
    private:
        void run();
        std::string today;
        timestamp m_begin;
        timestamp m_end;

        std::string m_beginStr;
        std::string m_endStr;

        int duration;
        OnCallback m_callback;
        std::atomic<TimerState> m_State;

        std::shared_ptr<std::thread> m_thread;
        std::atomic<bool> m_thread_finished = false;
    };
}
