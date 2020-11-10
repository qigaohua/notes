#ifndef __LINEAR_LIST_
#define __LINEAR_LIST_

#define MAX_SIZE 100

typedef int ElemType;

typedef struct _sqList {
	ElemType data[MAX_SIZE];
	int length;
}sqList_s, *sqList_p;

int initList(sqList_s **list);
int isFullList(sqList_p list);
int isEmptyList(sqList_p list);
int lengthList(sqList_p list);
int addListOfTail(sqList_p list, ElemType value);
int addList(sqList_p list, ElemType value, int i);
int getElemList(sqList_p list, int i, ElemType *e);
int deleteList(sqList_p list, int i, ElemType *e);
int clearList(sqList_p list);



#endif

