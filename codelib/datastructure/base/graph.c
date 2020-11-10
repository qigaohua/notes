#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"


//#define OrthogonalList
#define AdjList



#ifdef MATRIX

int locateGraph(matrixGraph *graph, vertexType c)
{
	int i;

	for (i = 0; i < graph->numVertex; i++) {
		if (graph->vertex[i] == c)
			return i;
	}

	return -1;
}

int printfGraph(matrixGraph *graph)
{
	int i, j;

	for(i = 0; i < graph->numVertex; i ++) {
		for (j = 0; j < graph->numVertex; j ++) {
			printf("%d ", graph->martix[i][j]);
		}
		printf("\n");
	}

	return 0;
}

/*创建邻接矩阵*/
int crateMatrixGraph(matrixGraph *graph)
{
	int i, j, k, w;
	vertexType x,y;

	printf("input numEages:");
	scanf("%d", &(graph->numEages));
	printf("input numVertex:");
	scanf("%d", &(graph->numVertex));

	if (graph->numVertex > MAX_VERTEX)
		return -1;

	for (i = 0; i < graph->numVertex; i++) {
		printf("input vertex:");
		graph->vertex[i] = getchar();
		while(graph->vertex[i] == '\n')
			graph->vertex[i] = getchar();
	}


	for (i = 0; i < graph->numVertex; i ++) {
		for (j = 0; j < graph->numVertex; j ++)
			graph->martix[i][j] = 0;
	}

	getchar();
	for (k = 0; k < graph->numEages; k ++) {
		printf("input vertex and w:\n");
		x = getchar();
		while(x == '\n')
			x = getchar();
		y = getchar();
		while(y == '\n')
			y = getchar();
		scanf("%d", &w); 
		//scanf("%c,%c,%d", &x, &y, &w);
		i = locateGraph(graph, x);
		j = locateGraph(graph, y);

		if (i == -1 || j == -1) {
			printf("input error\n");
			k --;
			continue;
		}

		/*无向图*/
		graph->martix[i][j] = w;
		graph->martix[j][i] = graph->martix[i][j];
	}

	return 0;
}
#endif

#ifdef AdjList
/*根据顶点返回在数组中的位置*/
int locateGraph(adjListGraph_p adjGraph, vertexType c)
{
	int i;

	for (i = 0; i < adjGraph->numVertex; i++) {
		if (adjGraph->graph[i].vertex == c)
			return i;
	}

	return -1;
}

int printfGraph(adjListGraph_p adjGraph)
{
	int i;
	edgeNode_p p1,p2;

	for(i = 0; i < adjGraph->numVertex; i ++) {
		printf("%c->", adjGraph->graph[i].vertex);
		p1 = adjGraph->graph[i].fistNode;
		while(p1) {
			p2 = p1->next;
			printf("%c(%d),", p1->vertex, p1->weight);
			p1 = p2;
		}
		printf("\n");
	}

	return 0;
}

/*深度优先遍历*/
int visited[MAX_VERTEX]; /*遍历标记*/
int DFS(adjListGraph_p adjGraph, int i)
{
	edgeNode_p e;
	int j;
	
	printf("vertex: %c\n", adjGraph->graph[i].vertex);
	visited[i] = 1;
	e = adjGraph->graph[i].fistNode;

	while(e) {
		j = locateGraph(adjGraph, e->vertex);
		if (!visited[j])
			DFS(adjGraph, j);

		e = e->next;
	}
	
	return 0;
}

int DFSAdjList(adjListGraph_p adjGraph)
{
	int i;

	for(i = 0; i < adjGraph->numVertex; i++)
		visited[i] = 0;

	for(i = 0; i < adjGraph->numVertex; i++) {
		if (!visited[i]) {
			DFS(adjGraph, i);
		}
	}

	return 0;
}

typedef struct _queue {
	int head, tail;
	vertexType vertex[MAX_VERTEX];
} Queue;

int isEmptyQueue(Queue *q)
{
	return q->head == q->tail;
}

int isFullQueue(Queue *q)
{
	return (q->head + 1)%MAX_VERTEX == q->tail;
}

int enqueue(Queue *q, vertexType v) 
{
	if (isFullQueue(q))
		return -1;
	
	q->vertex[q->head] = v;
	q->head = (q->head + 1)%MAX_VERTEX;
	return 0;
}

int dequeue(Queue *q, vertexType *v) 
{
	if (isEmptyQueue(q))
		return -1;
	
	*v = q->vertex[q->tail];
	q->tail = (q->tail + 1)%MAX_VERTEX;
	return 0;
}

int BFS(adjListGraph_p adjGraph, int i)
{
	int j;
	edgeNode_p e;
	Queue q;
	vertexType v;

	if (visited[i])
		return -1;
	memset(&q, 0 , sizeof(q));
	printf("vertex: %c\n", adjGraph->graph[i].vertex);
	visited[i] = 1;

	enqueue(&q, adjGraph->graph[i].vertex);
	while (!isEmptyQueue(&q)) {
		dequeue(&q, &v);
		j = locateGraph(adjGraph, v);
		if (j == -1) return -2;
		e = adjGraph->graph[j].fistNode;
		while(e) {
			j = locateGraph(adjGraph, e->vertex);
			if (j == -1) return -2;
			if (!visited[j]) {
				printf("vertex: %c\n", e->vertex);
				visited[j] = 1;
				enqueue(&q, e->vertex);
			}
			e = e->next;	
		}
	}
	
	return 0;
}


/*广度优先遍历*/
int BFSAdjList(adjListGraph_p adjGraph)
{
	int i;

	for(i = 0; i < adjGraph->numVertex; i++)
		visited[i] = 0;

	for(i = 0; i < adjGraph->numVertex; i++) {
		if (!visited[i]) {
			BFS(adjGraph, i);
		}
	}

	return 0;
}


int crateAdjListGraph(adjListGraph_p adjGraph)
{
	int i, j, k, w;
	vertexType x,y;
	edgeNode_p edgeNode;

	if (!adjGraph)
		return -1;

	printf("input numEages:");
	scanf("%d", &(adjGraph->numEdges));
	printf("input numVertex:");
	scanf("%d", &(adjGraph->numVertex));

	if (adjGraph->numVertex > MAX_VERTEX)
		return -1;

	for (i = 0; i < adjGraph->numVertex; i++) {
		printf("input vertex:");
		adjGraph->graph[i].vertex = getchar();
		while(adjGraph->graph[i].vertex == '\n')
			adjGraph->graph[i].vertex = getchar();
	}

	for (k = 0; k < adjGraph->numEdges; k ++) {
		printf("input vertex and weight\n");
		x = getchar();
		while(x == '\n')
			x = getchar();
		y = getchar();
		while(y == '\n')
			y = getchar();
		scanf("%d", &w); 
	
		/*邻接表*/
		i = locateGraph(adjGraph, x);
		j = locateGraph(adjGraph, y);

		if (i == -1 || j == -1) {
			printf("input error\n");
			k --;
			continue;
		}

		edgeNode = (edgeNode_p)malloc(sizeof(*edgeNode));
		if (!edgeNode)
			return -2;
		edgeNode->weight = w;
		edgeNode->vertex = y;

		edgeNode->next = adjGraph->graph[i].fistNode;
		adjGraph->graph[i].fistNode = edgeNode;

		/*无向图增加*/
#if 0
		edgeNode = (edgeNode_p)malloc(sizeof(*edgeNode));
		if (!edgeNode)
			return -2;
		edgeNode->weight = w;
		edgeNode->vertex = x;

		edgeNode->next = adjGraph->graph[j].fistNode;
		adjGraph->graph[j].fistNode = edgeNode;
#endif
#if 0		
		/*逆邻接表*/
		j = locateGraph(adjGraph, y);
		//j = locateGraph(adjGraph, y);

		if (j == -1) {
			printf("input error\n");
			k --;
			continue;
		}

		edgeNode = (edgeNode_p)malloc(sizeof(*edgeNode));
		if (!edgeNode)
			return -2;
		edgeNode->weight = w;
		edgeNode->vertex = x;

		edgeNode->next = adjGraph->graph[j].fistNode;
		adjGraph->graph[j].fistNode = edgeNode;
#endif
	}
	return 0;
}


/*边的结构*/
typedef struct _Edge {
	vertexType st; /*开始顶点*/
	vertexType end; /*结束顶点*/
	weightType weight; /*权重*/
} Edge;


/*最小生成树   prim（普里姆）算法*/
int Prim(adjListGraph_p adjGraph)
{
	int j, m, n;
	edgeNode_p e;
	Edge edge[adjGraph->numVertex];
	weightType min;
	int index;

	memset(edge, 0, sizeof(Edge) * adjGraph->numVertex);
	for (j = 0; j < adjGraph->numVertex; j++)
		edge[j].weight = INT_MAX;

	for (m = 0; m < (adjGraph->numVertex - 1); m ++) {
		min = INT_MAX;
		if (m == 0) index = m;
		e = adjGraph->graph[index].fistNode;
		edge[index].weight = -1;
		while (e) {
			j = locateGraph(adjGraph, e->vertex);
			if (edge[j].weight > e->weight) {
				edge[j].st = adjGraph->graph[index].vertex; 
				edge[j].end = e->vertex;
				edge[j].weight = e->weight;
			}
			e = e->next;
		}

		for (n = 0; n < adjGraph->numVertex; n++) {
			if (edge[n].weight != -1) {
				if (min > edge[n].weight) {
					index = n;
					min = edge[n].weight;
				}
			}		
		}	

		printf("%c---->%c   %d\n",  edge[index].st, edge[index].end, min);
	}

	return 0;
}


#endif

#ifdef OrthogonalList
int locateGraph(ortListGraph_p adjGraph, vertexType c)
{
	int i;

	for (i = 0; i < adjGraph->numVertex; i++) {
		if (adjGraph->graph[i].vertex == c)
			return i;
	}

	return -1;
}

int printfGraph(ortListGraph_p adjGraph)
{
	int i;
	edgeNode_p p1,p2;

	if (!adjGraph) return -1;
	printf("Out:\n");
	for(i = 0; i < adjGraph->numVertex; i ++) {
		printf("%c->", adjGraph->graph[i].vertex);
		p1 = adjGraph->graph[i].fistOut;
		while(p1) {
			p2 = p1->next;
			printf("%c(%d),", p1->vertex, p1->weight);
			p1 = p2;
		}
		printf("\n");
	}
	printf("In:\n");
	for(i = 0; i < adjGraph->numVertex; i ++) {
		printf("%c<-", adjGraph->graph[i].vertex);
		p1 = adjGraph->graph[i].fistIn;
		while(p1) {
			p2 = p1->againNext;
			printf("%c(%d),", p1->againVertex, p1->weight);
			p1 = p2;
		}
		printf("\n");
	}

	return 0;
}

int crateOrtListGraph(ortListGraph_p ortGraph)
{
	int i, j, k, w;
	vertexType x,y;
	edgeNode_p edgeNode;

	if (!ortGraph)
		return -1;

	printf("input numEages(int):");
	if (1 != scanf("%d", &(ortGraph->numEdges))) { printf("input error\n");return -2;}
	printf("input numVertex(int):");
	if (1 != scanf("%d", &(ortGraph->numVertex))) { printf("input error\n");return -2;}

	if (ortGraph->numVertex > MAX_VERTEX)
		return -1;

	for (i = 0; i < ortGraph->numVertex; i++) {
		printf("input vertex(char):");
		ortGraph->graph[i].vertex = getchar();
		while(ortGraph->graph[i].vertex == '\n')
			ortGraph->graph[i].vertex = getchar();
	}

	for (k = 0; k < ortGraph->numEdges; k ++) {
		printf("input vertex(char) and weight(int):\n");
		x = getchar();
		while(x == '\n') x = getchar();

		y = getchar();
		while(y == '\n') y = getchar();
		scanf("%d", &w); 
	
		i = locateGraph(ortGraph, x);
		j = locateGraph(ortGraph, y);

		if (i == -1 || j == -1) {
			printf("input error\n");
			k --;
			continue;
		}

		edgeNode = (edgeNode_p)malloc(sizeof(*edgeNode));
		if (!edgeNode)
			return -2;
		edgeNode->weight = w;

		edgeNode->vertex = y;
		edgeNode->next = ortGraph->graph[i].fistOut;
		ortGraph->graph[i].fistOut = edgeNode;


		edgeNode->againVertex = x;
		edgeNode->againNext = ortGraph->graph[j].fistIn;
		ortGraph->graph[j].fistIn = edgeNode;
	}
	return 0;
}

#endif



int main()
{
	adjListGraph_s g;

	memset(&g, 0, sizeof(g));
	crateAdjListGraph(&g);
	printf("打印图:\n");
	printfGraph(&g);
	printf("深度优先遍历:\n");
	DFSAdjList(&g);
	printf("广度优先遍历:\n");
	BFSAdjList(&g);
	printf("最小生成树:\n");
	Prim(&g);

	return 0;
}



