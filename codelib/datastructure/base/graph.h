#ifndef __GRAPH__H
#define __GRAPH__H

/*邻接矩阵*/
#define MAX_VERTEX 100
typedef char  vertexType; /*顶点类型*/
typedef int  weightType;
//int visited[MAX_VERTEX]; /*for 遍历标记*/

typedef struct _matrixGraph {
	vertexType vertex[MAX_VERTEX];
	int martix[MAX_VERTEX][MAX_VERTEX];	
	int numEages, numVertex;
}matrixGraph;


/*邻接表*/
/*边节点结构*/
typedef struct _edgeNode {
	vertexType vertex; /*顶点*/
	weightType weight; /*权重*/
	struct _edgeNode *next;
} edgeNode_s, *edgeNode_p;

/*数组0结构*/
typedef struct _vertexGraph {
	vertexType vertex;
	edgeNode_p fistNode;
}vertexGraph[MAX_VERTEX], vertexGraph_s;

typedef struct _adjListGraph {
	int numEdges, numVertex; /*顶点数与边数*/
	vertexGraph graph;
} adjListGraph_s, *adjListGraph_p;


#if 0
/*十字链表*/
typedef struct _edgeNode {
	vertexType vertex;
	vertexType againVertex;
	weightType weight;
	struct _edgeNode *next;
	struct _edgeNode *againNext;
} edgeNode_s, *edgeNode_p;

typedef struct _vertexGraph {
	vertexType vertex;
	edgeNode_p fistIn;
	edgeNode_p fistOut;
}vertexGraph[MAX_VERTEX], vertexGraph_s;

typedef struct _OrthogonalListGraph {
	int numEdges, numVertex;
	vertexGraph graph;
} ortListGraph_s, *ortListGraph_p;
#endif
#endif

