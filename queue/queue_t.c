#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "queue_t.h"

void my_init_packet(packet_t *pkt)
{
    memset(pkt, 0, sizeof(*pkt));
}

void my_packet_unref(packet_t *pkt)
{
	if (pkt->data != NULL) {
		free(pkt->data);
		pkt->data = NULL;
	}
    pkt->size = 0;
}

void my_free(void *arg)
{
    (arg ? free(arg) : NULL);
}

static int packet_queue_put_private(queue_t *q, packet_t *pkt)
{
    packet_list_t *pkt1;

    if (q->abort_request)
       return -2;

    if ((unsigned int)q->max_size <= (unsigned int)q->size) {
        printf("Exceed max packets or max size!\n");
        return -1;
    }

    pkt1 = (void *)malloc(sizeof(packet_list_t));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->size += pkt1->pkt.size + sizeof(*pkt1);

    pthread_cond_signal(q->cond);
    return 0;
}

int packet_queue_put(queue_t *q, packet_t *pkt)
{
    int ret;

    pthread_mutex_lock(q->mutex);
    ret = packet_queue_put_private(q, pkt);
    pthread_mutex_unlock(q->mutex);

	if (ret < 0)
		my_packet_unref(pkt);

    return ret;
}

int packet_queue_put_nullpacket(queue_t *q)
{
    packet_t pkt1, *pkt = &pkt1;
    my_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;

    return packet_queue_put(q, pkt);
}

int packet_queue_init(queue_t *q, int max_size)
{
    memset(q, 0, sizeof(queue_t));

    pthread_cond_init(&q->prv_cond, NULL);
    pthread_mutex_init(&q->prv_mutex, NULL);

    q->mutex = &q->prv_mutex;
    q->cond = &q->prv_cond;
    q->abort_request = 1;
    q->max_size = max_size;
    return 0;
}

void packet_queue_flush(queue_t *q)
{
    packet_list_t *pkt, *pkt1;

    pthread_mutex_lock(q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        my_packet_unref(&pkt->pkt);
        free(pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->size = 0;
    pthread_mutex_unlock(q->mutex);
}

void packet_queue_destroy(queue_t *q)
{
    packet_queue_flush(q);
    pthread_mutex_destroy(q->mutex);
    pthread_cond_destroy(q->cond);
}

void packet_queue_abort(queue_t *q)
{
    pthread_mutex_lock(q->mutex);
    q->abort_request = 1;
    pthread_cond_signal(q->cond);
    pthread_mutex_unlock(q->mutex);
}

void packet_queue_start(queue_t *q)
{
    pthread_mutex_lock(q->mutex);
    q->abort_request = 0;

    pthread_mutex_unlock(q->mutex);
}

/* return < 0 if aborted, 0 if no packet */
int packet_queue_get(queue_t *q, packet_t *pkt)
{
    packet_list_t *pkt1;
    int ret;

    pthread_mutex_lock(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -2;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt = pkt1->pkt;
            my_free(pkt1);
            ret = 1;
            break;
        } else {
            pthread_cond_wait(q->cond, q->mutex);
        }
    }
    pthread_mutex_unlock(q->mutex);
    return ret;
}
