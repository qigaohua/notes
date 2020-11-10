/**
 * @file test.c
 * @brief  位组的使用测试
 * @author qigaohua, qigaohua168@163.com
 * @version
 * @date 2020-06-10
 */


#include <stdio.h>
#include <string.h>
#include "bitset.h"

/*
 * 利用 Sieve of Eratosthenes 算法求素数
 */

/*打印10万以内的素数*/
#define MAX 100000

int main()
{
	char bitarray[BITNSLOTS(MAX)];
	int i, j;

	memset(bitarray, 0, BITNSLOTS(MAX));

	for(i = 2; i < MAX; i++) {
		if(!BITTEST(bitarray, i)) {
			printf("%d\n", i);
			for(j = i + i; j < MAX; j += i)
				BITSET(bitarray, j);
			}
	}

	return 0;
}
