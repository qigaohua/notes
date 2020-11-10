#ifndef __STACK__H
#define __STACH__H

typedef int ElemType;

typedef struct _stack {
	ElemType *top;
	ElemType *base;
	int stackSize;
} stack_s, *stack_p;


int createStack(stack_p stack, int stackSize);
int isFullStack(stack_p stack);
int isEmptyStack(stack_p stack);
int isLengthStack(stack_p stack);
int pushStack(stack_p stack, ElemType value);
int popStack(stack_p stack, ElemType *value);
int printfStack(stack_p stack);

#endif

