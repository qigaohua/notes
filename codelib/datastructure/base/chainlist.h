#ifndef __CHAIN_LIST_
#define __CHAIN_LIST_


typedef int ElemType;

typedef struct _node {
	ElemType data;
	struct _node *next;
}chainNode_s, *chainNode_p;


int initList(chainNode_s **head);
int lengthList(chainNode_p head);
int addListOfHead(chainNode_p head, ElemType value);
int addListOfTail(chainNode_p head, ElemType value);
int insertList(chainNode_p head, ElemType value, int i);
int getValueList(chainNode_p head, ElemType *value, int i);
int deleteList(chainNode_p head, int i);
int deleteListOfTail(chainNode_p head);
int freeList(chainNode_p head);
int printfList(chainNode_p head);


#endif

