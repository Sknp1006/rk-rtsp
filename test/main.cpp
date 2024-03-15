#include "rtspHandle.h"
#include <signal.h>
#include "logger.h"
#include "utils_time.h"

static int exit_sig = 0;

void exit_signal(int sig)
{
    exit_sig = 1;
}

int main(int argc, char **argv)
{
    commons::Log::init("./");
    std::string url = std::string(argv[1]);
    SPDLOG_DEBUG("url: {}", url.c_str());
    RTSPHandle handle = RTSPHandle(url);

    signal(SIGTERM, exit_signal);

    cv::Mat frame;
    while (!exit_sig)
    {
        if (handle.getFrame(frame))
        {
            SPDLOG_INFO("get frame: {} x {} -t {}", frame.size().width, frame.size().height, Utils::getCurrentTime().c_str());
            // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}