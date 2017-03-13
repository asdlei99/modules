#ifndef _QUEUE_T_H
#define _QUEUE_T_H

typedef struct packet_t {
    int size;
    char *data;
} packet_t;

typedef struct packet_list_t {
    packet_t pkt;
    struct packet_list_t *next;
} packet_list_t;

typedef struct queue_t {
    packet_list_t *first_pkt, *last_pkt;
    int max_size;
    int size;
    int abort_request;

    pthread_mutex_t *mutex;
    pthread_cond_t *cond;

    /* private */
    pthread_mutex_t prv_mutex;
    pthread_cond_t prv_cond;
} queue_t;


void    my_init_packet(packet_t *pkt);
void    my_packet_unref(packet_t *pkt);
void    my_free(void *arg);
int     packet_queue_put(queue_t *q, packet_t *pkt);
int     packet_queue_put_nullpacket(queue_t *q);
int     packet_queue_init(queue_t *q, int max_size);
void    packet_queue_flush(queue_t *q);
void    packet_queue_destroy(queue_t *q);
void    packet_queue_abort(queue_t *q);
void    packet_queue_start(queue_t *q);
int     packet_queue_get(queue_t *q, packet_t *pkt);

#endif /* _QUEUE_T_H */
