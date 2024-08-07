#include "utils_time.h"
#include <spdlog/spdlog.h>
/// @brief 将时间戳转换成对应格式的字符串
/// @param timestamp 时间戳
/// @param fmt 时间格式
/// @return 日期字符串
std::string Utils::timestamp2string(time_t timestamp, const char* fmt)
{
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), fmt);
    std::string date = ss.str();
    return date;
}
/// @brief 日期字符串转换成时间戳
/// @param str 时间字符串
/// @param fmt 时间格式
/// @return 时间戳
time_t Utils::string2timestamp(const char* str, const char* fmt)
{
    std::stringstream ss;
    ss << str;

    std::tm tm{};
    ss>>std::get_time(&tm, fmt);
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return std::chrono::system_clock::to_time_t(tp);
}
/// @brief 获取当前日期的字符串
/// @return 年月日字符串
std::string Utils::getCurrentDate()
{
    SPDLOG_TRACE("Utils::getCurrentDate()");
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string time = Utils::timestamp2string(now, "%Y-%m-%d");
    return time;
}
/// @brief 获取当前时间字符串
/// @return 时分秒字符串
std::string Utils::getCurrentTime()
{
    SPDLOG_TRACE("Utils::getCurrentTime()");
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string time = Utils::timestamp2string(now, "%Y-%m-%d %H:%M:%S");
    return time;
}
/// @brief 获取当前日期和时间字符串
Utils::Timer::Timer()
{
    SPDLOG_TRACE("Utils::Timer::Timer()");
    this->today = Utils::getCurrentDate();      // 获取今天的日期
    this->m_State = TimerState::WAITING;
}
/// @brief 计时器构造函数
/// @param Begin 
/// @param End 
Utils::Timer::Timer(const char* Begin, const char* End) : Timer()
{
    SPDLOG_TRACE("Utils::Timer::Timer(const char* Begin, const char* End)");
    this->m_beginStr = Begin;
    this->m_endStr = End;

    this->updateDate();
    this->formatTime();
}
/// @brief 计时器析构函数
Utils::Timer::~Timer()
{
    SPDLOG_TRACE("Utils::Timer::~Timer()");
}
/// @brief 开始计时器线程
void Utils::Timer::start()
{
    SPDLOG_TRACE("Utils::Timer::start()");
    this->m_thread = std::make_shared<std::thread>(&Utils::Timer::run, this);
}
/// @brief 计时器运行
void Utils::Timer::run()
{
    SPDLOG_TRACE("Utils::Timer::run()");
    this->formatTime();
    timestamp now;
    do
    {
        now = std::chrono::system_clock::now();
        if(this->updateDate()) { this->formatTime(); }
        if (now < this->m_begin)
        {
            SPDLOG_DEBUG("Utils::Timer::run() : now < m_begin");
            this->m_State = TimerState::WAITING;
            if (this->m_onWaitCallback) this->m_onWaitCallback();
        }
        else if (now >= this->m_begin && now < this->m_end)
        {
            SPDLOG_DEBUG("Utils::Timer::run() : now >= m_begin && now < m_end");
            this->m_State = TimerState::RUNNING;
            if (this->m_onRunCallback) this->m_onRunCallback();
        }
        else
        {
            SPDLOG_DEBUG("Utils::Timer::run() : now >= m_end");
            this->m_State = TimerState::STOPPED;
            if (this->m_onStopCallback) this->m_onStopCallback();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } while (this->m_State.load() != TimerState::QUIT);
    this->m_thread_finished = true;
}
/// @brief 计时器停止
void Utils::Timer::stop()
{
    SPDLOG_TRACE("Utils::Timer::stop()");

    SPDLOG_DEBUG("Utils::Timer::stop() stopping timer.");
    if (this->m_State.load() != TimerState::QUIT)
    {
        this->m_State.store(TimerState::QUIT);
        if (this->m_thread->joinable())
        {
            this->m_thread->join();
        }
    }
    SPDLOG_DEBUG("Utils::Timer::stop() timer stopped.");
}
/// @brief 获取计时器状态
/// @return 
Utils::TimerState Utils::Timer::getState() const
{
    SPDLOG_TRACE("Utils::Timer::getState()");
    return this->m_State.load();
}
/// @brief 获取开始时间
/// @return 
std::string Utils::Timer::getBeginTime() const
{
    return Utils::timestamp2string(std::chrono::system_clock::to_time_t(this->m_begin), "%Y-%m-%d %H:%M:%S");
}
/// @brief 获取结束时间
/// @return 
std::string Utils::Timer::getEndTime() const
{
    return Utils::timestamp2string(std::chrono::system_clock::to_time_t(this->m_end), "%Y-%m-%d %H:%M:%S");
}
/// @brief 初始化m_begin和m_end
void Utils::Timer::formatTime()
{
    SPDLOG_TRACE("Utils::Timer::formatTime()");
    std::string begin = fmt::format("{} {}", this->today, this->m_beginStr);
    std::string end = fmt::format("{} {}", this->today, this->m_endStr);
    this->m_begin = std::chrono::system_clock::from_time_t(Utils::string2timestamp(begin.c_str(), "%Y-%m-%d %H:%M:%S"));
    this->m_end = std::chrono::system_clock::from_time_t(Utils::string2timestamp(end.c_str(), "%Y-%m-%d %H:%M:%S"));
}
/// @brief 更新日期
/// @return 更新了日期返回true
bool Utils::Timer::updateDate()
{
    SPDLOG_TRACE("Utils::Timer::updateDate()");
    if (this->today != Utils::getCurrentDate())
    {
        this->today = Utils::getCurrentDate();
        return true;
    }
    return false;
}
