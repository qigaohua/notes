#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



void BuildGoodS(const char* pattern, size_t pattern_length, unsigned int* goods)
{
	unsigned int i, j, c;

	for(i = 0; i < pattern_length - 1; ++i)
	{
		goods[i] = pattern_length;
	}

	//初始化pattern最末元素的好后缀值
	goods[pattern_length - 1] = 1;

	//此循环找出pattern中各元素的pre值，这里goods数组先当作pre数组使用
	for(i = pattern_length -1, c = 0; i != 0; --i)
	{
		for(j = 0; j < i; ++j)
		{
			if(memcmp(pattern + i, pattern + j, (pattern_length - i) * sizeof(char)) == 0)
			{
				if(j == 0)
				{
					c = pattern_length - i;
				}
				else
				{
					if(pattern[i - 1] != pattern[j - 1])
					{
						goods[i - 1] = j - 1;
					}
				}
			}
		}
	}

    	//根据pattern中个元素的pre值，计算goods值
	for(i = 0; i < pattern_length - 1; ++i)
	{
		if(goods[i] != pattern_length)
		{
			goods[i] = pattern_length - 1 - goods[i];
		}
		else
		{
			goods[i] = pattern_length - 1 - i + goods[i];

			if(c != 0 && pattern_length - 1 - i >= c)
			{
				goods[i] -= c;
			}
		}
	}

	for(i = 0; i < pattern_length - 1; ++i)
	{
        printf("%d\t", goods[i]);
	}
    printf("\n");
}

void suffixes2(char *x, int m, int *suff)
{
    int i, q;
    suff[m-1]=m;
    for (i=m-2; i>=0; --i){
        q=i;
        while (q>=0 && x[q] == x[m-1-i+q])
            --q;
        printf("i = %d, q = %d\n", i, q);
        suff[i]=i-q;
    }
}

void suffixes(char *x, int m, int *suff) {
   int f, g, i;

   suff[m - 1] = m;
   g = m - 1;
   for (i = m - 2; i >= 0; --i) {
      if (i > g && suff[i + m - 1 - f] < i - g)
         suff[i] = suff[i + m - 1 - f];
      else {
         if (i < g)
            g = i;
         f = i;
         while (g >= 0 && x[g] == x[g + m - 1 - f])
            --g;
         suff[i] = f - g;
      }
   }
}

#define XSIZE 256
#define ASIZE 256
#define MAX(a, b)  ((a) > (b) ? (a) : (b))


void preBmBc(char *x, int m, int bmBc[]) {
   int i;

   for (i = 0; i < ASIZE; ++i)
      bmBc[i] = m;
   for (i = 0; i < m - 1; ++i)
      bmBc[x[i]] = m - i - 1;
}

void preBmGs(char *x, int m, int bmGs[]) {
   int i, j, suff[XSIZE];

   suffixes(x, m, suff);

   for (i = 0; i < m; ++i)
      bmGs[i] = m;
   j = 0;
   for (i = m - 1; i >= 0; --i)
      if (suff[i] == i + 1)
         for (; j < m - 1 - i; ++j)
            if (bmGs[j] == m)
               bmGs[j] = m - 1 - i;
   for (i = 0; i <= m - 2; ++i)
      bmGs[m - 1 - suff[i]] = m - 1 - i;
}


void BM(char *x, int m, char *y, int n) {
   int i, j, bmGs[XSIZE], bmBc[ASIZE];

   /* Preprocessing */
   preBmGs(x, m, bmGs);
   preBmBc(x, m, bmBc);

   /* Searching */
   j = 0;
   while (j <= n - m) {
      for (i = m - 1; i >= 0 && x[i] == y[i + j]; --i);
      if (i < 0) {
          printf(">>>>find: %d\n", j);
         j += bmGs[0];
      }
      else
         j += MAX(bmGs[i], bmBc[y[i + j]] - m + 1 + i);
   }
}


int main(int argc, char *argv[])
{
    // unsigned int suff[64] = {0};
    // unsigned int bmGs[64] = {0};
    char *pattern = "BCDBCDABCDABCD";
    // // BuildGoodS("BCDBCDABCDABCD", 14, goods);
    // int i;
	// for(i = 0; i < 14; ++i)
	// {
    //     printf("%c\t", pattern[i]);
	// }
    // printf("\n");

    // suffixes(pattern, 14, suff);
	// for(i = 0; i < 14; ++i)
	// {
    //     printf("%d\t", suff[i]);
	// }
    // printf("\n");

    // preBmGs(pattern, 14, bmGs);
	// for(i = 0; i < 14; ++i)
	// {
    //     printf("%d\t", bmGs[i]);
	// }
    // printf("\n");

    BM("BCDF", 4, pattern, 14);

    return 0;
}
