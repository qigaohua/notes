#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "util.h"

#define MAX_RING_CAPACITY 1024

typedef struct __dns_file__ {
    //WORD32 id;
    char dns_file[1024];
} DNS_FILE;

typedef struct ring_queue{
    DNS_FILE ring_array[MAX_RING_CAPACITY];
    int front;
    int rear;
    pthread_mutex_t q_lock;
}RING_QUEUE;


int init_queue(RING_QUEUE *ring);
int is_queue_full(RING_QUEUE *ring);
int is_queue_empty(RING_QUEUE *ring);
int enqueue(RING_QUEUE *ring, DNS_FILE item);
int dequeue(RING_QUEUE *ring, DNS_FILE *item);

#endif