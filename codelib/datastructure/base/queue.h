#ifndef __QUEUE__H
#define __QUEUE__H

#include <pthread.h>

typedef int ElemType;


typedef struct _queue {
	ElemType *base;
	int front;
	int rear;
	pthread_mutex_t queueLock;
	int queueSize;
}queue_s, *queue_p;



#endif

