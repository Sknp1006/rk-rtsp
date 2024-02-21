#include "PacketQueue.h"
namespace av
{
	void packet_queue_init(Queue *q)
	{
		memset(q, 0, sizeof(Queue));
		q->mutex = SDL_CreateMutex();
		q->cond = SDL_CreateCond();
		q->size = 0;
		q->nb_packets = 0;
		q->first_pkt = NULL;
		q->last_pkt = NULL;

		q->quit = 0;
	}

	int packet_queue_put(Queue *q, AVPacket *pkt)
	{
		AVPacketList *pkt1;
		pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
		if (!pkt1)
			return -1;

		if (av_packet_ref(&pkt1->pkt, pkt) < 0)
		{
			return -1;
		}

		pkt1->next = NULL;

		SDL_LockMutex(q->mutex);

		if (!q->last_pkt)
			q->first_pkt = pkt1;
		else
			q->last_pkt->next = pkt1;
		q->last_pkt = pkt1;
		q->nb_packets++;
		q->size += pkt1->pkt.size;
		SDL_CondSignal(q->cond);

		SDL_UnlockMutex(q->mutex);
		return 0;
	}

	static int packet_queue_get(Queue *q, AVPacket *pkt, int block)
	{
		AVPacketList *pkt1;
		int ret = -1;

		SDL_LockMutex(q->mutex);
		for (;;)
		{
			if (q->quit)
			{
				break;
			}

			pkt1 = q->first_pkt;
			if (pkt1)
			{
				q->first_pkt = pkt1->next;
				if (!q->first_pkt)
					q->last_pkt = NULL;
				q->nb_packets--;
				q->size -= pkt1->pkt.size;

				av_packet_ref(pkt, &pkt1->pkt);

				av_packet_unref(&pkt1->pkt);
				av_free(pkt1);

				ret = 1;
				break;
			}
			else if (!block)
			{
				ret = 0;
				break;
			}
			else
			{
				SDL_CondWait(q->cond, q->mutex);
			}
		}
		SDL_UnlockMutex(q->mutex);

		return ret;
	}
	static void packet_queue_quit(Queue *q)
	{
		SDL_LockMutex(q->mutex);

		// set quit
		q->quit = 1;

		SDL_CondSignal(q->cond);

		SDL_UnlockMutex(q->mutex);
	}
	static void packet_queue_flush(Queue *q)
	{
		AVPacketList *pkt, *pkt1;

		SDL_LockMutex(q->mutex);
		for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1)
		{
			pkt1 = pkt->next;
			// av_free_packet(&pkt->pkt);
			av_freep(&pkt);
		}
		q->last_pkt = NULL;
		q->first_pkt = NULL;
		q->nb_packets = 0;
		q->size = 0;
		SDL_UnlockMutex(q->mutex);
	}

	PacketQueue::PacketQueue()
	{
		packet_queue_init(&queue);
	}
	PacketQueue::~PacketQueue()
	{
	}
	int PacketQueue::put(AVPacket *pkt)
	{
		return packet_queue_put(&queue, pkt);
	}
	int PacketQueue::get(AVPacket *pkt, bool block)
	{
		return packet_queue_get(&queue, pkt, block ? 1 : 0);
	}
	void PacketQueue::flush()
	{
		packet_queue_flush(&queue);
	}
	void PacketQueue::quit()
	{
		packet_queue_quit(&queue);
	}
	int PacketQueue::size()
	{
		return queue.size;
	}
	int PacketQueue::numberOfPackets()
	{
		return queue.nb_packets;
	}
}