#include "queue.h"

int init_queue(RING_QUEUE *ring)
{
    if (NULL == ring) {
        return -1;
    }
    
    ring->front = 0;
    ring->rear = 0;

    pthread_mutex_init(&ring->q_lock, NULL);
    
    return 0;
}

int is_queue_full(RING_QUEUE *ring)
{
    if ((ring->rear + 1)%MAX_RING_CAPACITY == ring->front) {
        return 1;  //full
    } else {
        return 0;
    }
}

int is_queue_empty(RING_QUEUE *ring)
{
    pthread_mutex_lock(&ring->q_lock);
    if (ring->front == ring->rear) {
        pthread_mutex_unlock(&ring->q_lock);
        return 1;  //empty
    } else {
        pthread_mutex_unlock(&ring->q_lock);
        return 0;
    }
}

int __is_queue_empty__(RING_QUEUE *ring)
{
    if (ring->front == ring->rear) {      
        return 1;  //empty
    } else {
        return 0;
    }
}

int enqueue(RING_QUEUE *ring, DNS_FILE item)
{
    pthread_mutex_lock(&ring->q_lock);
    if (1 == is_queue_full(ring)) {
        pthread_mutex_unlock(&ring->q_lock);
        return -1;
    }
    ring->ring_array[ring->rear] = item;
    ring->rear = (ring->rear + 1)%MAX_RING_CAPACITY;
    
    pthread_mutex_unlock(&ring->q_lock);
    return 0;
}

int dequeue(RING_QUEUE *ring, DNS_FILE *item)
{
    pthread_mutex_lock(&ring->q_lock);
    if (1 == __is_queue_empty__(ring)) {
        pthread_mutex_unlock(&ring->q_lock);
        return -1;
    }
    *item = ring->ring_array[ring->front];
    ring->front = (ring->front + 1)%MAX_RING_CAPACITY;
    
    pthread_mutex_unlock(&ring->q_lock);
    return 0;
}