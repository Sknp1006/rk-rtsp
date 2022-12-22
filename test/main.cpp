#include "rtspHandle.h"
#include <signal.h>
#include "logger.h"
#include "utils_time.h"

static int exit_sig = 0;

void thread_func(std::string url)
{
    RTSPHandle handle = RTSPHandle(url);
    cv::Mat frame;
    while (!exit_sig)
    {
        if (handle.getFrame(frame))
        {
            std::cout << "Thread id:" << std::this_thread::get_id() << " " << "get frame: " << frame.size().width << " x " << frame.size().height << std::endl;
        }
    }
}

void exit_signal(int sig)
{
    exit_sig = 1;
}

int main(int argc, char **argv)
{
    commons::Log::init("./");
    std::string url = std::string(argv[1]);
    SPDLOG_DEBUG("url: {}", url.c_str());

    signal(SIGTERM, exit_signal);

    const int n = 8;
    std::array<std::thread, n> threads;

    for (auto &t : threads)
    {
        t = std::thread(thread_func, url);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}