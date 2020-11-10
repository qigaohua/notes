#include <stdio.h>
#include <stdlib.h>
#include "stack.h"


/*创建栈*/
int createStack(stack_p stack, int stackSize)
{
	stack->base = (ElemType *)malloc(sizeof(ElemType) * stackSize);
	if (NULL == stack->base) {
		perror("malloc error");
		return -1;
	}

	stack->top = stack->base;
	stack->stackSize = stackSize;

	return 0;
}

/*判断栈是否为满*/
int isFullStack(stack_p stack)
{
	if (!stack)
		return -1;

	return (stack->top - stack->base) == stack->stackSize;
}

/*判断栈是否为空*/
int isEmptyStack(stack_p stack)
{
	if (!stack)
		return -1;

	return stack->top == stack->base;
}

/*返回栈的长度*/
int isLengthStack(stack_p stack)
{
	if (!stack)
		return -1;

	return stack->top - stack->base;
}

/*向栈中放入元素*/
int pushStack(stack_p stack, ElemType value)
{
	if (!stack)
		return -1;

	if (isFullStack(stack))
		return -1;

	*(stack->top) = value;
	(stack->top) ++;

	return 0;
}


/*向栈中取出元素*/
int popStack(stack_p stack, ElemType *value)
{
	if (!stack)
		return -1;

	if (isEmptyStack(stack))
		return -1;

	*value = *--(stack->top);

	return 0;
}

/*打印*/
int printfStack(stack_p stack)
{
	int i;

	if (!stack)
		return -1;

	for (i = 0; i < isLengthStack(stack); i++)
		printf("%d\t", *((stack->base) + i));

	printf("\n");

	return 0;
}


#if 1

/*测试*/
#define STACK_SIZE 100

int main(int argc, char **argv)
{
	int i;
	ElemType e;
	stack_s sk;

	createStack(&sk, STACK_SIZE);

	printf("It is empty stack ?\t%s\n", isEmptyStack(&sk) ? "Yes" : "No");
	for (i = 0; i < 10; i ++)
		pushStack(&sk, i);
	printf("It is empty stack ?\t%s\n", isEmptyStack(&sk) ? "Yes" : "No");

	printf("length: %d\n", isLengthStack(&sk));
	printfStack(&sk);

	popStack(&sk, &e);
	printf("length: %d   value: %d\n", isLengthStack(&sk), e);
	printfStack(&sk);

	popStack(&sk, &e);
	printf("length: %d   value: %d\n", isLengthStack(&sk), e);
	printfStack(&sk);

	popStack(&sk, &e);
	printf("length: %d   value: %d\n", isLengthStack(&sk), e);
	printfStack(&sk);
	return 0;
}


#endif
