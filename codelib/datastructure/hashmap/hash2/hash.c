/**
 * @file hash.c
 * @brief 平方探测法散列表实现
 * @author qigaohua, qigaohua168@163.com
 * @version 0.1
 * @date 2018-12-14
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "hash.h"


#define Error(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt": %s\r\n", __FILE__, __LINE__, ##__VA_ARGS__, \
            strerror(errno))
#define Warn(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define Info(fmt, ...) \
    fprintf(stdout, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)


// 是否是素数
static int is_prime(int num)
{
    if (num == 2 || num == 3)
        return 1;

    if (num % 6 != 1 && num % 6 != 5)
        return 0;

    for (int i = 5; i*i <= num; i += 6) {
        if (num % i == 0 || num % (i+2) == 0)
            return 0;
    }

    return 1;
}

static int next_prime(int n)
{
    int state=is_prime(n);
    while(!state)
        state=is_prime(++n);

    return n;
}


static unsigned int BKDRHash(const char *str, unsigned int tablesize)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF) % tablesize;
}


HashTable hashtable_create(int tablesize, hash_func hash,
        free_element_func free_element)
{
    HashTable table;

    if (tablesize < HASHTABLE_SIZE_MIN) {
        Warn("Hashtable size too small");
        return NULL;
    }

    Info("Start create hashtable ...");

    table = calloc(1, sizeof (struct hashtable));
    if (!table) {
        Error("Call calloc() failed");
        return NULL;
    }

    table->tablesize = next_prime(tablesize);
    table->list = calloc(table->tablesize, sizeof(List));
    if (!table->list) {
        Error("Call calloc() failed");
        goto err;
    }

    int i = 0;
    for (;i < table->tablesize; i++) {
        table->list[i] = calloc(1, sizeof(struct listnode));
        if (!table->list[i]) {
            Error("Call calloc() failed");
            goto err;
        }
        table->list[i]->st = Empty;
    }

    if (hash) table->hash = hash;
    else table->hash = BKDRHash; // default

    if (free_element) table->free_element = free_element;
    // else table->free_element = free; // default

    Info("Start create hashtable success !!!");
    return table;
err:
    Info("Start create hashtable failed !!!");
    if (table) free(table);
    return NULL;
}



void hashtable_destroy(HashTable table)
{
    int i = 0;

    if (!table) return;

    for (; i < table->tablesize; i++) {
        if (table->list[i]->st == Legitimate) {
            free(table->list[i]->key);
            if (table->list[i]->element && table->free_element)
                table->free_element(table->list[i]->element);
        }
        free(table->list[i]);
    }

    free(table->list);
    free(table);
}


Possion hashtable_find(HashTable table, const char *key)
{
    Possion pos = NULL;
    unsigned int curIndex, i = 0;

    curIndex = table->hash(key, table->tablesize);
    pos = table->list[curIndex];

    while (pos->st != Empty) {
        // Warn("%s:%s  %u", pos->key, key, i);
        if (pos->key && !strncmp(pos->key, key, strlen(pos->key)))
            break;

        curIndex += 2 * ++i - 1;
        if (curIndex >= table->tablesize)
            curIndex -= table->tablesize;
        pos = table->list[curIndex];
    }

    return pos;
}


// 最好不用删除
int hashtable_delete(HashTable table, const char *key)
{
    Possion pos;

    pos = hashtable_find(table, key);
    if (pos->st == Legitimate) {
        free(pos->key);
        pos->key = NULL;
        if (pos->element && table->free_element)
            table->free_element(pos->element);
        pos->element = NULL;
        pos->st = Delete;
    }
    else
       Warn("Key(%s) not exists", key);

    return 0;
}


int hashtable_insert(HashTable table, const char *key, void *element)
{
    Possion pos = NULL;
    unsigned int curIndex, i = 0;

    curIndex = table->hash(key, table->tablesize);
    pos = table->list[curIndex];

    while (pos->st == Legitimate) {
        // Warn("%s:%s  %u", pos->key, key, i);
        if (pos->key && !strncmp(pos->key, key, strlen(pos->key)))
            break;

        curIndex += 2 * ++i - 1;
        if (curIndex >= table->tablesize)
            curIndex -= table->tablesize;
        pos = table->list[curIndex];
    }

    if (pos->st != Legitimate) {
        pos->key = strdup(key);
        pos->element = element;
        pos->st = Legitimate;
    }
    else
       Warn("Key(%s) already exists", key);

    return 0;
}








/******************** test *************************/

#if 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define PALIGN   "%15s: %6.4f sec\n"
#define NKEYS   1024*1024
//#define NKEYS   10240
#define DEBUG 0

static double epoch_double(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (t.tv_usec * 1.0) / 1000000.0;
}

int main(int argc, char * argv[])
{
    HashTable table;
    double t1, t2 ;
    int i ;
    int nkeys ;
    char * buffer ;
    char * val ;

    nkeys = (argc>1) ? (int)atoi(argv[1]) : NKEYS ;
    printf("%15s: %d\n", "values", nkeys);
    buffer = (char *)malloc(9 * nkeys);

    //                10000000
    table = hashtable_create(2097100, NULL, NULL);
    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        sprintf(buffer + i * 9, "%08x", i);
    }
    t2 = epoch_double();
    printf(PALIGN, "initialization", t2 - t1);



    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        printf(">>>>>>>>>>>>>>>>>>>>>>>>> Insert %d\n", i);
        hashtable_insert(table, buffer + i*9, buffer +i*9);
    }
    t2 = epoch_double();
    printf(PALIGN, "adding", t2 - t1);


    sleep(5);



    t1 = epoch_double();
    int cnt = 20;
    int y = 0;
    Possion pos;
    for(i = 0; i < nkeys; i++) {
        pos = hashtable_find(table, buffer + i*9);
        if (cnt > 0) {
            cnt --;
            if(pos->st == Legitimate)
                printf("hash_get: key=%s, val=%s\n", buffer + i*9, (char*)pos->element);
        }
#if DEBUG>1
        if (pos->st == Legitimate) {
            printf("exp[%s] got[%s]\n", buffer+i*9, (char *)pos->element);
            if (pos->element && strcmp((char *)pos->element, buffer+i*9)) {
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG got[%s] exp[%s]\n", (char *)pos->element, buffer+i*9);
                // break;
            }
        }
        else
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG %s\n",  buffer+i*9);

#endif
    }
    t2 = epoch_double();
    printf(PALIGN, "lookup", t2 - t1);



    sleep(5);



    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        hashtable_delete(table, buffer + i*9);
    }
    t2 = epoch_double();
    printf(PALIGN, "delete", t2 - t1);




    sleep(5);


    for(i = 0; i < nkeys; i++) {
        pos = hashtable_find(table, buffer + i*9);
        if (cnt > 0) {
            cnt --;
            if(pos->st == Legitimate)
                printf("hash_get: key=%s, val=%s\n", buffer + i*9, (char*)pos->element);
        }
#if DEBUG>1
        if (pos->st == Legitimate) {
            printf("exp[%s] got[%s]\n", buffer+i*9, (char *)pos->element);
            if (pos->element && strcmp((char *)pos->element, buffer+i*9)) {
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG got[%s] exp[%s]\n", (char *)pos->element, buffer+i*9);
                // break;
            }
        }
        else
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG %s\n",  buffer+i*9);

#endif
    }
    t2 = epoch_double();
    printf(PALIGN, "lookup", t2 - t1);



    sleep(5);


    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        printf(">>>>>>>>>>>>>>>>>>>>>>>>> Insert %d\n", i);
        hashtable_insert(table, buffer + i*9, buffer +i*9);
    }
    t2 = epoch_double();
    printf(PALIGN, "adding", t2 - t1);



    sleep(5);


    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        pos = hashtable_find(table, buffer + i*9);
        if (cnt > 0) {
            cnt --;
            if(pos->st == Legitimate)
                printf("hash_get: key=%s, val=%s\n", buffer + i*9, (char*)pos->element);
        }
#if DEBUG>1
        if (pos->st == Legitimate) {
            printf("exp[%s] got[%s]\n", buffer+i*9, (char *)pos->element);
            if (pos->element && strcmp((char *)pos->element, buffer+i*9)) {
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG got[%s] exp[%s]\n", (char *)pos->element, buffer+i*9);
                // break;
            }
        }
        else
                printf(">>>>>>>>>>>>>>>>>>>>>>>>> WRONG %s\n",  buffer+i*9);

#endif
    }
    t2 = epoch_double();
    printf(PALIGN, "free", t2 - t1);


    sleep(5);


    t1 = epoch_double();
    hashtable_destroy(table);
    t2 = epoch_double();
    printf(PALIGN, "free", t2 - t1);

    free(buffer);
    return 0 ;

}
#endif
