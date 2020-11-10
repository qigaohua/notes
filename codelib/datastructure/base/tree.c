#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"


/*创建节点*/
int createTreeNode(treeNode_s **bTree, ElemType value)
{
	*bTree = (treeNode_s *)malloc(sizeof(treeNode_s));
	if (!bTree) {
		perror("malloc error");
		return -1;
	}	

	memset(*bTree, 0, sizeof(treeNode_s));
	(*bTree)->value = value;

	return 0;
}

/*查找节点*/
treeNode_p findTreeNode(treeNode_p bTree, ElemType value)
{
	if (!bTree) return NULL;

	if (bTree->value == value)
		return bTree;
   	else if (bTree->value > value)
		return findTreeNode(bTree->lChild, value);
	else
		return findTreeNode(bTree->rChild, value);	
}


/*插入节点*/
int insertTreeNode(treeNode_s **bTree, ElemType value)
{
	treeNode_p node = NULL;

	if (!*bTree) {
		createTreeNode(&node, value);
		if (!node) return -1;
		*bTree = node;
	} else {
		/*允许重复*/
		if ((*bTree)->value >= value)
			insertTreeNode(&((*bTree)->lChild), value);
		else
			insertTreeNode(&((*bTree)->rChild), value);	
	}

	return 0;
}

/*删除节点，分为四种情况*/
int deleteTreeNode(treeNode_s **bTree, ElemType value)
{
	treeNode_p pnode = *bTree, r, p;

	if (!bTree) return -1;
	
	if (pnode->value > value)
		deleteTreeNode(&(pnode->lChild), value);
	else if (pnode->value < value)
		deleteTreeNode(&(pnode->rChild), value);
	else {	
		/*被删节点没有左右子节点*/
		if (!(pnode->lChild) && !(pnode->rChild)) 
			*bTree = NULL;	
		/*被删节点没有左子节点*/
		else if (!(pnode->lChild) && pnode->rChild) 
		 	*bTree = pnode->rChild;
		/*被删节点没有右子节点*/
		else if (!(pnode->rChild) && pnode->lChild)
			*bTree = pnode->lChild;	
		/*被删节点有左右子节点*/
		else {
			r = pnode->rChild;
			if (!(r->lChild)) {
				r->lChild = pnode->lChild;
			} else {
				while(r->lChild) {
					p = r;
					r = r->lChild;
				}
				p->lChild = r->rChild;
				r->lChild = pnode->lChild;
				r->rChild = pnode->rChild;
			}	
			*bTree = r;
		}
		free(pnode);
	}

	return 0;
}

/*返回当前节点数量*/
int countTreeNode(treeNode_p bTree)
{
	if (!bTree) return 0;

	return 1 + countTreeNode(bTree->lChild) + countTreeNode(bTree->rChild);
}

/*打印数据*/
int printfTree(treeNode_p bTree)
{
	if (bTree) {
		printfTree(bTree->lChild);
		printf("%d\t", bTree->value);
		printfTree(bTree->rChild);
	}

	return 0;
}

#if 1

/*test*/
int main()
{
	int i;
	treeNode_p tree = NULL;
	treeNode_p node = NULL;

	for(i = 0; i < 10 ; i += 2)
		insertTreeNode(&tree, i);

	printfTree(tree);
	printf("\n");

	for(i = 1; i < 10 ; i += 2)
		insertTreeNode(&tree, i);

	printfTree(tree);
	printf("\n");

	printf("current count: %d\n", countTreeNode(tree));

	node = findTreeNode(tree, 8);
	printf("find value: %d\n", node->value);

	printf("\n 删除0:\t");
	deleteTreeNode(&tree, 0);
	printfTree(tree);
	insertTreeNode(&tree, 0);

	printf("\n 删除1:\t");
	deleteTreeNode(&tree, 1);
	printfTree(tree);
	insertTreeNode(&tree, 1);

	printf("\n 删除2:\t");
	deleteTreeNode(&tree, 2);
	printfTree(tree);
	insertTreeNode(&tree, 2);

	printf("\n 删除3:\t");
	deleteTreeNode(&tree, 3);
	printfTree(tree);
	insertTreeNode(&tree, 3);


	printf("\n 删除4:\t");
	deleteTreeNode(&tree, 4);
	printfTree(tree);
	insertTreeNode(&tree, 4);


	printf("\n 删除5:\t");
	deleteTreeNode(&tree, 5);
	printfTree(tree);
	insertTreeNode(&tree, 5);

	printf("\n 删除6:\t");
	deleteTreeNode(&tree, 6);
	printfTree(tree);
	printf("\n");
	insertTreeNode(&tree, 6);
	return 0;
}


#endif



