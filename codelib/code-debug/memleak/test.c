#include <stdlib.h>  
#include <stdio.h>  
#include <unistd.h>  
#include "memleak.h"  

/*
test.c:38: dbg_free: NULL
A str
test.c:45: m: 6, c: 2, r: 4, f: 3, mem: 296
***** test.c:46: heap dump start
(alloc: test.c:29 size: 270)
(alloc: test.c:32 size: 0)
(alloc: test.c:33 size: 0)
(alloc: test.c:34 size: 10)
(alloc: test.c:40 size: 10)
(alloc: test.c:41 size: 6)
***** test.c:46: heap dump end
*/

int main()  
{  
		void *a,*b,*c;  
		char *s;  
		dbg_init(10);  
		dbg_catch_sigsegv();  
		a = malloc(1000);  
		b = malloc(30);  
		a = realloc(a, 150);  
		c = calloc(90, 3);  
		b = realloc(b, 0);  

		malloc(0);  
		calloc(0, 10);  
		realloc(0, 10);  

		realloc(a, 0);  

		free(0);  

		s = strdup("A string.");  
		s = strndup(s, 5);  

		puts(s);  

		dbg_mem_stat();  
		dbg_heap_dump("");  
		return 0;  
}  
