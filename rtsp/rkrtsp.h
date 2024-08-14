#ifndef H_RKRTSP
#define H_RKRTSP

#include <stdint.h>

#ifndef API_EXPORT
#define API_EXPORT __attribute__((visibility("default")))
typedef int64_t LONG64;
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    struct ImageData
    {
        unsigned char *data;
        int width;
        int height;
        int channels;
        int type;

        ~ImageData()
        {
            delete[] data;
        }
    };
    API_EXPORT LONG64 CreateRKRTSP(const char *rtsp_url);
    API_EXPORT void ReleaseRKRTSP(LONG64 ctx);

    API_EXPORT int GetFrame(LONG64 ctx, ImageData &frame);
    API_EXPORT void Stop(LONG64 ctx);
    API_EXPORT bool IsRunning(LONG64 ctx);
#ifdef __cplusplus
}
#endif

#endif // H_RKRTSP