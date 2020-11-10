#include <stdio.h>
#include <stdlib.h>
#include "chainlist.h"

/*初始化链式线性表*/
int initList(chainNode_s **head)
{
	*head = (chainNode_s *)malloc(sizeof(chainNode_s));
	if (NULL == *head) {
		perror("malloc error");
		return -1;
	}

	(*head)->data = 0;
	(*head)->next = NULL;

	return 0;
}

/*可以将头结点的数设置为线性表的长度，此没有实现*/
int lengthList(chainNode_p head)
{
	return head->data;
}

/*在线性表头部添加节点*/
int addListOfHead(chainNode_p head, ElemType value)
{
	chainNode_p node;

	if (head == NULL)
		return -1;

	node = (chainNode_p)malloc(sizeof(*node));
	if (NULL == node) {
		perror("malloc error");
		return -1;
	}

	node->data = value;
	if (NULL == head->next) {
		node->next = NULL;
		head->next = node;
	} else {
		node->next = head->next;
		head->next = node;
	}
		
	return 0;
}

/*在线性表尾部添加节点*/
int addListOfTail(chainNode_p head, ElemType value)
{
	chainNode_p node;
	chainNode_p p;

	if (head == NULL)
		return -1;

	node = (chainNode_p)malloc(sizeof(*node));
	if (NULL == node) {
		perror("malloc error");
		return -1;
	}

	node->data = value;
	node->next = NULL;

	if (NULL == head->next) {
		node->next = NULL;
		head->next = node;
	}

	p = head->next;
	while(p->next && (p = p->next));
	p->next = node;
		
	return 0;
}

/*在线性表 i 处插入节点*/
int insertList(chainNode_p head, ElemType value, int i)
{
	chainNode_p node;
	chainNode_p p;

	if (head == NULL || i <= 0)
		return -1;

	node = (chainNode_p)malloc(sizeof(*node));
	if (NULL == node) {
		perror("malloc error");
		return -1;
	}

	node->data = value;
	if (NULL == head->next) {
		node->next = NULL;
		head->next = node;
	} else {
		p = head;
		/*得到插入节点处的上一个节点*/
		while(p->next && ((--i) > 0))
			p = p->next;
		node->next = p->next;
		p->next = node;
	}
		
	return 0;
}

/*取出节点 i 的数据*/
int getValueList(chainNode_p head, ElemType *value, int i)
{
	chainNode_p p;

	if (head == NULL || i <= 0)
		return -1;

	p = head;
	while(p->next && ((i --) > 0))
		p = p->next;
		
	/*i 超出线性表长度*/
	if (i > 0)
		return -1;

	*value = p->data;

	return 0;
}

/*删除 节点i, 当i = 1时，即删除头部节点*/
int deleteList(chainNode_p head, int i)
{
	chainNode_p p;

	if (head == NULL)
		return -1;

	p = head;
	while(p->next && ((--i) > 0))
		p = p->next;
		
	if (i > 0 || p->next == NULL)
		return -1;

	free(p->next);
	p->next = p->next->next;

	return 0;
}

/*删除尾部节点*/
int deleteListOfTail(chainNode_p head)
{
	chainNode_p p;

	if (head == NULL)
		return -1;

	p = head;
	while(p->next->next && (p = p->next));

	p->next = NULL;
	free(p->next);
		
	return 0;
}

/*释放线性表*/
int freeList(chainNode_p head)
{
	chainNode_p p1, p2;

	if (head == NULL)
		return -1;

	p1 = head->next;
	while(p1) { 
		p2 = p1->next;
		free(p1);
		p1 = p2;
	}

	free(head);
		
	return 0;
}

int printfList(chainNode_p head)
{
	chainNode_p p;

	p = head->next;
	while(p) {
		printf("%d\t", p->data);
		p = p->next;
	}
	printf("\n");
	return 0;
}

#if 1
/*测试*/
int main()
{
	int i;
	ElemType e ;
	chainNode_p head = NULL ;

	initList(&head);

	for(i = 0; i < 10; i++)
		insertList(head, i, i+1);

	printfList(head);

	getValueList(head, &e, 1);
	printf("head: %d\n", e);

	getValueList(head, &e, 6);
	printf("list_6: %d\n", e);

	addListOfHead(head, 0);
	addListOfTail(head, 10);
	printfList(head);

	deleteListOfTail(head);
	printfList(head);

	deleteList(head, 5);
	printfList(head);

	freeList(head);
	return 0;
}

#endif



