#include "rtsp/rtspHandle.h"

int main(int argc, char **argv)
{
    std::string url = std::string(argv[1]);
    RTSPHandle handle = RTSPHandle(url);

    while (true)
    {
        cv::Mat frame;
        if (handle.getFrame(frame))
        {
            printf("Frame: %d x %d\n", frame.cols, frame.rows);
        }
    }
}