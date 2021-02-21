/* Copyright (C) 2013 Open Information Security Foundation
 *
 * You can copy, redistribute or modify this Program under the terms of
 * the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/**
 * \defgroup utilpool Pool
 *
 * @{
 */

/**
 * \file
 *
 * \author Victor Julien <victor@inliniac.net>
 *
 * Pool utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "util-pool.h"
#include "util-pool-thread.h"

PoolThread *PoolThreadInit(int threads, uint32_t size, uint32_t prealloc_size, uint32_t elt_size,  void *(*Alloc)(void), int (*Init)(void *, void *), void *InitData,  void (*Cleanup)(void *), void (*Free)(void *))
{
    PoolThread *pt = NULL;
    int i;

    if (threads <= 0) {
        LOGD("error");
        goto error;
    }

    pt = malloc(sizeof(*pt));
    if (unlikely(pt == NULL)) {
        LOGD("memory alloc error");
        goto error;
    }

    LOGD("size %d", threads);
    pt->array = malloc(threads * sizeof(PoolThreadElement));
    if (pt->array == NULL) {
        LOGD("memory alloc error");
        goto error;
    }
    pt->size = threads;

    for (i = 0; i < threads; i++) {
        PoolThreadElement *e = &pt->array[i];

        PoolMutexInit(&e->lock, NULL);
        PoolMutexLock(&e->lock);
//        LOGD("size %u prealloc_size %u elt_size %u Alloc %p Init %p InitData %p Cleanup %p Free %p",
//                size, prealloc_size, elt_size,
//                Alloc, Init, InitData, Cleanup, Free);
        e->pool = PoolInit(size, prealloc_size, elt_size, Alloc, Init, InitData, Cleanup, Free);
        PoolMutexUnlock(&e->lock);
        if (e->pool == NULL) {
            LOGD("error");
            goto error;
        }
    }

    return pt;
error:
    if (pt != NULL)
        PoolThreadFree(pt);
    return NULL;
}

/**
 *
 */
int PoolThreadGrow(PoolThread *pt, uint32_t size, uint32_t prealloc_size, uint32_t elt_size,  void *(*Alloc)(void), int (*Init)(void *, void *), void *InitData,  void (*Cleanup)(void *), void (*Free)(void *)) {
    void *ptmp;
    size_t newsize;
    PoolThreadElement *e = NULL;

    if (pt == NULL || pt->array == NULL) {
        LOGE("pool grow failed");
        return -1;
    }

    newsize = pt->size + 1;
    LOGD("newsize %"PRIuMAX, (uintmax_t)newsize);

    ptmp = realloc(pt->array, (newsize * sizeof(PoolThreadElement)));
    if (ptmp == NULL) {
        free(pt->array);
        pt->array = NULL;
        LOGE("pool grow failed");
        return -1;
    }
    pt->array = ptmp;

    pt->size = newsize;

    e = &pt->array[newsize - 1];
    memset(e, 0x00, sizeof(*e));
    PoolMutexInit(&e->lock, NULL);
    PoolMutexLock(&e->lock);
    e->pool = PoolInit(size, prealloc_size, elt_size, Alloc, Init, InitData, Cleanup, Free);
    PoolMutexUnlock(&e->lock);
    if (e->pool == NULL) {
        LOGE("pool grow failed");
        return -1;
    }

    return (int)(newsize - 1);
}

int PoolThreadSize(PoolThread *pt)
{
    if (pt == NULL)
        return -1;
    return (int)pt->size;
}

void PoolThreadFree(PoolThread *pt)
{
    int i;

    if (pt == NULL)
        return;

    if (pt->array != NULL) {
        for (i = 0; i < (int)pt->size; i++) {
            PoolThreadElement *e = &pt->array[i];
            PoolMutexLock(&e->lock);
            PoolFree(e->pool);
            PoolMutexUnlock(&e->lock);
            PoolMutexDestroy(&e->lock);
        }
        free(pt->array);
    }
    free(pt);
}

void *PoolThreadGetById(PoolThread *pt, uint16_t id)
{
    void *data = NULL;

    if (pt == NULL || id >= pt->size)
        return NULL;

    PoolThreadElement *e = &pt->array[id];
    PoolMutexLock(&e->lock);
    data = PoolGet(e->pool);
    PoolMutexUnlock(&e->lock);
    if (data) {
        PoolThreadReserved *did = data;
        *did = id;
    }

    return data;
}

void PoolThreadReturn(PoolThread *pt, void *data)
{
    PoolThreadReserved *id = data;

    if (pt == NULL || *id >= pt->size)
        return;

    LOGD("returning to id %u", *id);

    PoolThreadElement *e = &pt->array[*id];
    PoolMutexLock(&e->lock);
    PoolReturn(e->pool, data);
    PoolMutexUnlock(&e->lock);
}

#ifdef UNITTESTS
struct PoolThreadTestData {
    PoolThreadReserved res;
    int abc;
};

static void *PoolThreadTestAlloc(void)
{
    void *data = malloc(sizeof(struct PoolThreadTestData));
    return data;
}

static
int PoolThreadTestInit(void *data, void *allocdata)
{
    if (!data)
        return 0;

    memset(data,0x00,sizeof(allocdata));
    struct PoolThreadTestData *pdata = data;
    pdata->abc = *(int *)allocdata;
    return 1;
}

static
void PoolThreadTestFree(void *data)
{
}

static int PoolThreadTestInit01(void)
{
    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, NULL, NULL, NULL, NULL);
    if (pt == NULL)
        return 0;

    PoolThreadFree(pt);
    return 1;
}

static int PoolThreadTestInit02(void)
{
    int i = 123;

    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL);
    if (pt == NULL)
        return 0;

    PoolThreadFree(pt);
    return 1;
}

static int PoolThreadTestGet01(void)
{
    int result = 0;
    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, NULL, NULL, NULL, NULL);
    if (pt == NULL)
        return 0;

    void *data = PoolThreadGetById(pt, 3);
    if (data == NULL) {
        printf("data == NULL: ");
        goto end;
    }

    struct PoolThreadTestData *pdata = data;
    if (pdata->res != 3) {
        printf("res != 3, but %d: ", pdata->res);
        goto end;
    }

    result = 1;
end:
    PoolThreadFree(pt);
    return result;
}

static int PoolThreadTestGet02(void)
{
    int i = 123;
    int result = 0;

    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL);
    if (pt == NULL)
        return 0;

    void *data = PoolThreadGetById(pt, 3);
    if (data == NULL) {
        printf("data == NULL: ");
        goto end;
    }

    struct PoolThreadTestData *pdata = data;
    if (pdata->res != 3) {
        printf("res != 3, but %d: ", pdata->res);
        goto end;
    }

    if (pdata->abc != 123) {
        printf("abc != 123, but %d: ", pdata->abc);
        goto end;
    }

    result = 1;
end:
    PoolThreadFree(pt);
    return result;
}

static int PoolThreadTestReturn01(void)
{
    int i = 123;
    int result = 0;

    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL);
    if (pt == NULL)
        return 0;

    void *data = PoolThreadGetById(pt, 3);
    if (data == NULL) {
        printf("data == NULL: ");
        goto end;
    }

    struct PoolThreadTestData *pdata = data;
    if (pdata->res != 3) {
        printf("res != 3, but %d: ", pdata->res);
        goto end;
    }

    if (pdata->abc != 123) {
        printf("abc != 123, but %d: ", pdata->abc);
        goto end;
    }

    if (pt->array[3].pool->outstanding != 1) {
        printf("pool outstanding count wrong %u: ",
                pt->array[3].pool->outstanding);
        goto end;
    }

    PoolThreadReturn(pt, data);

    if (pt->array[3].pool->outstanding != 0) {
        printf("pool outstanding count wrong %u: ",
                pt->array[3].pool->outstanding);
        goto end;
    }


    result = 1;
end:
    PoolThreadFree(pt);
    return result;
}

static int PoolThreadTestGrow01(void)
{
    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, NULL, NULL, NULL, NULL);
    if (pt == NULL)
        return 0;

    if (PoolThreadGrow(pt,
                       10, 5, 10, PoolThreadTestAlloc, NULL, NULL, NULL, NULL) < 0) {
        PoolThreadFree(pt);
        return 0;
    }

    PoolThreadFree(pt);
    return 1;
}

static int PoolThreadTestGrow02(void)
{
    int i = 123;

    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL);
    if (pt == NULL)
        return 0;

    if (PoolThreadGrow(pt,
                       10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL) < 0) {
        PoolThreadFree(pt);
        return 0;
    }

    PoolThreadFree(pt);
    return 1;
}

static int PoolThreadTestGrow03(void)
{
    int i = 123;
    int result = 0;

    PoolThread *pt = PoolThreadInit(4, /* threads */
                                    10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL);
    if (pt == NULL)
        return 0;

    if (PoolThreadGrow(pt,
                       10, 5, 10, PoolThreadTestAlloc, PoolThreadTestInit, &i, PoolThreadTestFree, NULL) < 0) {
        PoolThreadFree(pt);
        return 0;
    }

    void *data = PoolThreadGetById(pt, 4);
    if (data == NULL) {
        printf("data == NULL: ");
        goto end;
    }

    struct PoolThreadTestData *pdata = data;
    if (pdata->res != 4) {
        printf("res != 5, but %d: ", pdata->res);
        goto end;
    }

    if (pdata->abc != 123) {
        printf("abc != 123, but %d: ", pdata->abc);
        goto end;
    }

    if (pt->array[4].pool->outstanding != 1) {
        printf("pool outstanding count wrong %u: ",
                pt->array[4].pool->outstanding);
        goto end;
    }

    PoolThreadReturn(pt, data);

    if (pt->array[4].pool->outstanding != 0) {
        printf("pool outstanding count wrong %u: ",
                pt->array[4].pool->outstanding);
        goto end;
    }


    result = 1;
end:
    PoolThreadFree(pt);
    return result;
}

#endif

void PoolThreadRegisterTests(void)
{
#ifdef UNITTESTS
    UtRegisterTest("PoolThreadTestInit01", PoolThreadTestInit01);
    UtRegisterTest("PoolThreadTestInit02", PoolThreadTestInit02);

    UtRegisterTest("PoolThreadTestGet01", PoolThreadTestGet01);
    UtRegisterTest("PoolThreadTestGet02", PoolThreadTestGet02);

    UtRegisterTest("PoolThreadTestReturn01", PoolThreadTestReturn01);

    UtRegisterTest("PoolThreadTestGrow01", PoolThreadTestGrow01);
    UtRegisterTest("PoolThreadTestGrow02", PoolThreadTestGrow02);
    UtRegisterTest("PoolThreadTestGrow03", PoolThreadTestGrow03);
#endif
}

/**
 * @}
 */
