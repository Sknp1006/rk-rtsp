#pragma once
#include <thread>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_types.h>
#include <SDL2/SDL_name.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_config.h>
}

namespace av
{
    typedef struct Queue
    {
        AVPacketList *first_pkt, *last_pkt;
        int nb_packets;
        int size;
        SDL_mutex *mutex;
        SDL_cond *cond;

        int quit;
    } Queue;

    class PacketQueue
    {
        public:
            PacketQueue();
            ~PacketQueue();

            int put(AVPacket *pkt);
            int get(AVPacket *pkt, bool block);
            void flush();
            void quit();
            int size();
            int numberOfPackets();

        private:
            Queue queue;
    };
}