#ifndef __TREE_H_
#define __TREE_H_

typedef int ElemType;

typedef struct _treeNode {
	ElemType value;
	struct _treeNode *parent;
	struct _treeNode *lChild;
	struct _treeNode *rChild;
}treeNode_s, *treeNode_p;


int createTreeNode(treeNode_s **bTree, ElemType value);
treeNode_p findTreeNode(treeNode_p bTree, ElemType value);
int insertTreeNode(treeNode_s **bTree, ElemType value);
int deleteTreeNode(treeNode_s **bTree, ElemType value);
int countTreeNode(treeNode_p bTree);
int printfTree(treeNode_p bTree);



#endif
