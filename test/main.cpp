#include "rtspHandle.h"
#include <signal.h>
#include "logger.h"
#include "utils_time.h"

#include <SDL2/SDL.h>
#include <chrono>

#include "rknn.h"

#define FRAME_RATE 30

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

// 将 cv::Mat 转换为 SDL_Surface
SDL_Surface *matToSurface(const cv::Mat &mat)
{
    // 确定像素格式
    Uint32 rmask, gmask, bmask, amask;
    if (mat.channels() == 1)
    {
        rmask = gmask = bmask = 0;
        amask = 0;
    }
    else if (mat.channels() == 3)
    {
        rmask = 0x00ff0000;  // Red
        gmask = 0x0000ff00;  // Green
        bmask = 0x000000ff;  // Blue
        amask = 0;
    }
    else if (mat.channels() == 4)
    {
        rmask = 0x00ff0000;  // Red
        gmask = 0x0000ff00;  // Green
        bmask = 0x000000ff;  // Blue
        amask = 0xff000000;  // Alpha
    }
    else
    {
        return nullptr; // 不支持的通道数
    }

    // 创建 SDL_Surface
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        (void *)mat.data, mat.cols, mat.rows, mat.channels() * 8, mat.step,
        rmask, gmask, bmask, amask);

    if (surface == nullptr)
    {
        SDL_Log("SDL_CreateRGBSurfaceFrom failed: %s\n", SDL_GetError());
    }

    return surface;
}

void exit_signal(int sig)
{
    exit_sig = 1;
}

int sdl_loop(std::string url)
{
    // 窗口大小
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    RKNNHandle rknnHandle = RKNNHandle("./model/RK3588/yolov5s-640-640.rknn");

    // 初始化
    if (SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (NULL == window)
    {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (NULL == renderer)
    {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    RTSPHandle *handle;
    try
    {
        handle = new RTSPHandle(url);
    }
    catch(const std::exception& e)
    {
        SPDLOG_ERROR("RTSPHandle error: {}", e.what());
        delete handle;
        return -1;
    }
    
    cv::Mat frame;
    SDL_Event event;

    while (!exit_sig)
    {
        auto start = std::chrono::system_clock::now();
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                exit_sig = 1;
            }
        }

        if (handle->getFrame(frame))
        {
            rknnHandle.infer(frame);
            SDL_Surface *surface = matToSurface(frame);
            if (surface)
            {
                SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);

                if (texture)
                {
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, NULL, NULL);
                    SDL_RenderPresent(renderer);
                    SDL_DestroyTexture(texture);
                }
            }
        }
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (duration.count() < 1000 / FRAME_RATE) {
            SDL_Delay(1000 / FRAME_RATE - duration.count());
        }
    }
    handle->stop();

    // 销毁窗口和渲染器
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main(int argc, char **argv)
{
    commons::Log::init("./");
    std::string url = std::string(argv[1]);
    SPDLOG_DEBUG("url: {}", url.c_str());

    signal(SIGTERM, exit_signal);

    {
        // 多线程测试
        // const int n = 8;
        // std::array<std::thread, n> threads;
        // for (auto &t : threads)
        // {
        //     t = std::thread(thread_func, url);
        // }
        // for (auto &t : threads)
        // {
        //     t.join();
        // }
    }

    // 可视化测试
    sdl_loop(url);

    return 0;
}