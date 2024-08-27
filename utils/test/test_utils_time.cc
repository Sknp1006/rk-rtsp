#include <gtest/gtest.h>
#include "utils_time.h"

TEST(UtilsTimeTest, Timestamp2String) {
    time_t timestamp = 1609459200; // 2021-01-01 00:00:00
    std::string expected = "2021-01-01";
    std::string result = Utils::timestamp2string(timestamp, "%Y-%m-%d");
    EXPECT_EQ(result, expected);
}

TEST(UtilsTimeTest, String2Timestamp) {
    const char* dateStr = "2021-01-01 00:00:00";
    time_t expected = 1609459200; // 2021-01-01 00:00:00
    time_t result = Utils::string2timestamp(dateStr, "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(result, expected);
}

TEST(UtilsTimeTest, GetCurrentDate) {
    std::string currentDate = Utils::getCurrentDate();
    // Since we cannot predict the exact current date, we just check the format
    EXPECT_EQ(currentDate.size(), 10);
    EXPECT_EQ(currentDate[4], '-');
    EXPECT_EQ(currentDate[7], '-');
}

TEST(UtilsTimeTest, GetCurrentTime) {
    std::string currentTime = Utils::getCurrentTime();
    // Since we cannot predict the exact current time, we just check the format
    EXPECT_EQ(currentTime.size(), 19);
    EXPECT_EQ(currentTime[4], '-');
    EXPECT_EQ(currentTime[7], '-');
    EXPECT_EQ(currentTime[10], ' ');
    EXPECT_EQ(currentTime[13], ':');
    EXPECT_EQ(currentTime[16], ':');
}

TEST(UtilsTimeTest, TimerStateTransition) {
    auto onWaitCallback = []() {
        printf("Timer is waiting\n");
    };
    auto onRunCallback = []() {
        printf("Timer is running\n");
    };
    auto onStopCallback = []() {
        printf("Timer is stopped\n");
    };

    std::string start = Utils::timestamp2string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 2, "%Y-%m-%d %H:%M:%S");
    std::string end = Utils::timestamp2string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 5, "%Y-%m-%d %H:%M:%S");
    Utils::Timer timer(start.c_str(), end.c_str());
    timer.setOnWaitCallback(onWaitCallback);
    timer.setOnRunCallback(onRunCallback);
    timer.setOnStopCallback(onStopCallback);

    timer.start();
    EXPECT_EQ(timer.getState(), Utils::TimerState::WAITING);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(timer.getState() == Utils::TimerState::RUNNING);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_TRUE(timer.getState() == Utils::TimerState::STOPPED);

    timer.stop();
    EXPECT_EQ(timer.getState(), Utils::TimerState::QUIT);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}