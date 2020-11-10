#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linearlist.h"

int initList(sqList_s **list)
{
	*list = (sqList_s *)malloc(sizeof(sqList_s));
	if (NULL == *list) {
		perror("malloc error");
		return -1;
	}

	memset((*list)->data, 0, MAX_SIZE);
	(*list)->length = 0;

	return 0;
}


int isFullList(sqList_p list)
{
	return list->length == (MAX_SIZE - 1);
}

int isEmptyList(sqList_p list)
{
	return list->length == 0;
}

int lengthList(sqList_p list)
{
	return list->length;
}

int addListOfTail(sqList_p list, ElemType value)
{
	if (list == NULL)
		return -1;

	if (isFullList(list))
		return -1;

	list->data[list->length ++] = value;

	return 0;
}

int addList(sqList_p list, ElemType value, int i)
{
	int k;

	if (list == NULL)
		return -1;

	if (isFullList(list))
		return -1;

	for(k = list->length - 1 ; k >= i; k --)
		list->data[k + 1] = list->data[k];

	list->data[i] = value;
	list->length ++;

	return 0;
}

int getElemList(sqList_p list, int i, ElemType *e)
{
	if (NULL == list || i < 0 || i > list->length)
		return -1;

	*e = list->data[i];
	
	return 0;
}

int deleteList(sqList_p list, int i, ElemType *e)
{
	int k;

	if (NULL == list || i < 0 || i > list->length)
		return -1;

	*e = list->data[i];
	
	for(k = i; k < list->length - 1; k++)
		list->data[k] = list->data[k+1];

	list->length --;

	return 0;
}


int clearList(sqList_p list)
{
	if (list == NULL)
		return -1;

	memset(list->data, 0, MAX_SIZE);
	list->length = 0;

	return 0;
}

int freeList(sqList_p list)
{
	free(list);

	return 0;
}

int printfList(sqList_p list)
{
	int i;

	for (i = 0; i < list->length; i++)
		printf("%d\t", list->data[i]);
	
	printf("\n");

	return 0;
}

#if 1
/*测试*/
int main()
{
	int i;
	ElemType e ;
	sqList_p l = NULL ;

	initList(&l);

	printf("It is empty?  %s\n", (isEmptyList(l) ? "yes" : "no"));
	for(i = 0; i < MAX_SIZE - 5; i++)
		addListOfTail(l, i);

	printf("It is full?  %s\n", isFullList(l)==1 ? "yes" : "no");

	getElemList(l, 18, &e);
	printf("list[18] is %d\n", e);

	printf("length: %d\n", lengthList(l));
	printfList(l);
	addListOfTail(l, 100);
	printf("length: %d\n", lengthList(l));
	printfList(l);
	addList(l, 99, 4);
	printf("length: %d\n", lengthList(l));
	printfList(l);

	deleteList(l, 5, &e);
	printf("value: %d  length: %d\n", e, lengthList(l));
	printfList(l);

	freeList(l);
	return 0;
}

#endif



