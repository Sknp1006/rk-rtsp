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

    using timestamp = std::chrono::time_point<std::chrono::system_clock>;

    enum class TimerState
    {
        WAITING = -1,
        RUNNING = 0,
        STOPPED = 1,
        QUIT = 2
    };
    using OnCallback = std::function<void()>;
    class Timer
    {
    public:
        Timer();
        Timer(const char* Begin, const char* End);
        ~Timer();

        void start();
        void stop();
        TimerState getState() const;
        std::string getBeginTime() const;
        std::string getEndTime() const;

        void setOnWaitCallback(OnCallback callback) { m_onWaitCallback = callback; }
        void setOnRunCallback(OnCallback callback) { m_onRunCallback = callback; }
        void setOnStopCallback(OnCallback callback) { m_onStopCallback = callback; }
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

        OnCallback m_onWaitCallback;
        OnCallback m_onRunCallback;
        OnCallback m_onStopCallback;
        std::atomic<TimerState> m_State;

        std::shared_ptr<std::thread> m_thread;
        std::atomic<bool> m_thread_finished = false;
    };
}
