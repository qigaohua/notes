#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 插入排序
int insert_sort(int *array, int length)
{
	int i, j, tmp;

	if (NULL == array)
		return -1;

	if (length == 0 || length == 1)
		return 0;

	for (i = 1; i < length; i ++) {
		tmp = array[i];
		/*
		j = i - 1;
		while(j>=0&&array[j]>tmp) {
			array[j+1] = array[j];
			j --;
		}
		array[j+1] = tmp
		*/
		for (j = i - 1; j >= 0; j --) {
			if (array[j] > tmp)
				array[j + 1] = array[j];
			else
				break;
		}
		array[j + 1] = tmp;
	}

	return 0;
}


// 希尔排序
int shell_sort(int *array, int length, int step)
{
	int i, j, tmp;
	int gap = length;

	if (NULL == array || step == 2)
		return -1;

	if (length == 0 || length == 1)
		return 0;

	while (gap > 1){
		gap = gap / step + 1;
		for (i = gap; i < length; i += gap) {
			tmp = array[i];
			for (j = i - gap; j >= 0; j -= gap) {
				if (array[j] > tmp)
					array[j + gap] = array[j];
				else
					break;
			}
			array[j + gap] = tmp;
		}

	}
	return 0;
}


// 快速排序
int quick_sort(int *array, int start, int end)
{
	int right, left;
	int tmp;

	if (NULL == array)
		return -1;

	left = start;
	right = end;

	/*递归退出判断*/
	if (left < right) {
		tmp = array[left];

		while (left < right) {
			while (left < right && array[right] >= tmp)
				right --;
			array[left] = array[right];

			while (left < right && array[left] <= tmp)
				left ++;
			array[right] = array[left];
		}

		array[right] = tmp;
		quick_sort(array, start, left - 1);
		quick_sort(array, right + 1, end);
	}

	return 0;
}


/*合拼有序数组*/
int merge_array(int *array, int start, int middle, int end, int *temp)
{
	int i, j, k = 0;

	i = start;
	j = middle + 1;

	while(i <= middle && j <= end) {
		if (array[i] <= array[j])
			temp[k ++] = array[i ++];
		else
			temp[k ++] = array[j ++];
	}

	while (i <= middle)
		temp[k ++] = array[i ++];

	while (j <= end)
		temp[k ++] = array[j ++];

	for (i = 0; i < k; i ++)
		array[start + i] = temp[i];

	return 0;
}


/*将需要排序的数据递归对半拆分, 再进行合拼*/
int _merge_sort(int *array, int start, int end, int *temp)
{
	int middle;

	if (start < end) {
		middle = (end + start) / 2;
		_merge_sort(array, start, middle, temp);
		_merge_sort(array, middle + 1, end, temp);
		merge_array(array, start, middle, end, temp);
	}

	return 0;
}

// 归并排序
int mergeSort(int *array, int length)
{
	int *temp;

	if (NULL == array || length <= 0)
		return -1;

	temp = (int *)malloc(sizeof(int) * length);
	if (NULL == temp) {
		perror("malloc error");
		return -1;
	}
	memset(temp, 0, sizeof(int) * length);
	_merge_sort(array, 0, length - 1, temp);

	free(temp);

	return 0;
}

/*i的父节点，左子节点，右子节点*/
#define PARENT(i) ((i) >> 1)
#define LEFTCHILD(i) ((i) << 1)
#define RIGHTCHILD(i) (((i) << 1) + 1)

/*交换数组p1与p2的位置*/
int exchange(int *array, int p1, int p2)
{
	int temp;

	if (NULL == array)
		return -1;

	temp = array[p1];
	array[p1] = array[p2];
	array[p2] = temp;

	return 0;
}

/*维持最大堆的性质*/
int maxHeapify(int *array, int pos, int length)
{
	int maxPos, leftChild, rightChild;

	if (NULL == array || pos > length)
		return -1;

	maxPos = pos;
	leftChild = LEFTCHILD(pos);
	rightChild = RIGHTCHILD(pos);

	if (leftChild <= length && array[leftChild] > array[maxPos])
		maxPos = leftChild;
	if (rightChild <= length && array[rightChild] > array[maxPos])
		maxPos = rightChild;

	/*递归判断*/
	if (maxPos != pos) {
		exchange(array, pos, maxPos);
		maxHeapify(array, maxPos, length);
	}

	return 0;
}

// 堆排序
int heap_sort(int *array, int length)
{
	int i, arraySize;;

	if (NULL == array)
		return -1;

	arraySize = length;
	/*构建大型堆*/
    for (i = arraySize/2; i >= 1; i --) {
		maxHeapify(array, i, arraySize);
	}

	/*下面开始每次都取出堆里面堆顶位置的元素,再次进行堆性质维护*/
	for (i = arraySize; i >= 2; i --) {
		exchange(array, 1, i);
		arraySize --;
		maxHeapify(array, 1, arraySize);
	}

	//array[0] = arraySize;

	return 0;
}



int main(int argc, char **argv)
{
	int i, length;
    int buf[5] = {
         16, 32, 8, 12, 13
    };
	// int buf[] = {112, 5,1,11, 34, 10, 111,44,33,23,56,77,88,43,41,2,4,19,13,3,8,6,0,9};
	//int buf[10]={4,1,3,2,16,9,10,14,8,7};

	length = sizeof(buf)/sizeof(int);
	// heap_sort(buf-1, length);
    quick_sort(buf, 0, length - 1);

	for(i = 0; i < length; i++)
		printf("%d\t ", buf[i]);
	printf("\n");
	return 0;
}




