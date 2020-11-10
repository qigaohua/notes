#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/*环形队列*/

/*创建队列*/
int createQueue(queue_s *queue, int queueSize)
{
	queue->base = (ElemType *)malloc(sizeof(ElemType) * queueSize);
	if (NULL == queue->base) {
		perror("malloc error");
		return -1;
	}

	queue->front = queue->rear = 0;
	queue->queueSize = queueSize;
	pthread_mutex_init(&(queue->queueLock), NULL);

	return 0;
}

int __isFullQueue(queue_p queue)
{
	return (queue->front + 1) % queue->queueSize == queue->rear;
}

int isFullQueue(queue_p queue)
{
	int ret = 0;
	pthread_mutex_lock(&queue->queueLock);
	if ((queue->front + 1) % queue->queueSize == queue->rear)
		ret = 1;
	pthread_mutex_unlock(&queue->queueLock);
	return ret;
}

int __isEmptyQueue(queue_p queue)
{
	return queue->front == queue->rear;
}

int isEmptyQueue(queue_p queue)
{
	int ret = 0;
	pthread_mutex_lock(&queue->queueLock);
	if (queue->front == queue->rear)
		ret = 1;
	pthread_mutex_unlock(&queue->queueLock);
	return ret;
}

int pushQueue(queue_p queue, ElemType value)
{
	if (NULL == queue)
		return -1;

	if (__isFullQueue(queue))
		return -1;

	pthread_mutex_lock(&queue->queueLock);
	queue->base[queue->front] = value;
	queue->front = (queue->front + 1) % queue->queueSize;
	pthread_mutex_unlock(&queue->queueLock);

	return 0;
}

int popQueue(queue_p queue, ElemType *value)
{
	if (NULL == queue)
		return -1;

	if (__isEmptyQueue(queue))
		return -1;

	pthread_mutex_lock(&queue->queueLock);
	*value = queue->base[queue->rear];
	queue->rear = (queue->rear + 1) % queue->queueSize;
	pthread_mutex_unlock(&queue->queueLock);

	return 0;
}

#if 1

int main()
{
	int i;
	ElemType e;
	queue_s q;

	createQueue(&q, 20);
	
	printf("It is empty ?\t%s\n", isEmptyQueue(&q) ? "yes" : "no");
	for(i = 0; i < 20; i ++)
		pushQueue(&q, i);
	printf("It is full ?\t%s\n", isFullQueue(&q) ? "yes" : "no");

	popQueue(&q, &e);
	printf("value: %d\n", e);

	popQueue(&q, &e);
	printf("value: %d\n", e);

	popQueue(&q, &e);
	printf("value: %d\n", e);

	return 0;
}


#endif


