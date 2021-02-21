/* Copyright (C) 2007-2010 Open Information Security Foundation
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
 * ::Pool are an effective way to maintain a set of ready to use
 * structures.
 *
 * To create a ::Pool, you need to use PoolInit(). You can
 * get an item from the ::Pool by using PoolGet(). When you're
 * done with it call PoolReturn().
 * To destroy the ::Pool, call PoolFree(), it will free all used
 * memory.
 *
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>
#include "util-pool.h"

static int PoolMemset(void *pitem, void *initdata)
{
    Pool *p = (Pool *) initdata;

    memset(pitem, 0, p->elt_size);
    return 1;
}

/**
 * \brief Check if data is preallocated
 * \retval 0 if not inside the prealloc'd block, 1 if inside */
static int PoolDataPreAllocated(Pool *p, void *data)
{
    ptrdiff_t delta = data - p->data_buffer;
    if ((delta < 0) || (delta > p->data_buffer_size)) {
        return 0;
    }
    return 1;
}

/** \brief Init a Pool
 *
 * PoolInit() creates a ::Pool. The Alloc function must only do
 * allocation stuff. The Cleanup function must not try to free
 * the PoolBucket::data. This is done by the ::Pool management
 * system.
 *
 * \param size
 * \param prealloc_size
 * \param elt_size Memory size of an element
 * \param Alloc An allocation function or NULL to use a standard malloc
 * \param Init An init function or NULL to use a standard memset to 0
 * \param InitData Init data
 * \param Cleanup a free function or NULL if no special treatment is needed
 * \param Free free func
 * \retval the allocated Pool
 */
Pool *PoolInit(uint32_t size, uint32_t prealloc_size, uint32_t elt_size,
        void *(*Alloc)(void), int (*Init)(void *, void *), void *InitData,
        void (*Cleanup)(void *), void (*Free)(void *))
{
    Pool *p = NULL;

    if (size != 0 && prealloc_size > size) {
        LOGE("size error");
        goto error;
    }
    if (size != 0 && elt_size == 0) {
        LOGE("size != 0 && elt_size == 0");
        goto error;
    }
    if (elt_size && Free) {
        LOGE("elt_size && Free");
        goto error;
    }

    /* setup the filter */
    p = malloc(sizeof(Pool));
    if (unlikely(p == NULL)) {
        LOGE("malloc error");
        goto error;
    }

    memset(p,0,sizeof(Pool));

    p->max_buckets = size;
    p->preallocated = prealloc_size;
    p->elt_size = elt_size;
    p->data_buffer_size = prealloc_size * elt_size;
    p->Alloc = Alloc;
    p->Init = Init;
    p->InitData = InitData;
    p->Cleanup = Cleanup;
    p->Free = Free;
    if (p->Init == NULL) {
        p->Init = PoolMemset;
        p->InitData = p;
    }

    /* alloc the buckets and place them in the empty list */
    uint32_t u32 = 0;
    if (size > 0) {
        PoolBucket *pb = calloc(size, sizeof(PoolBucket));
        if (unlikely(pb == NULL)) {
            LOGE("alloc error");
            goto error;
        }
        memset(pb, 0, size * sizeof(PoolBucket));
        p->pb_buffer = pb;
        for (u32 = 0; u32 < size; u32++) {
            /* populate pool */
            pb->next = p->empty_stack;
            pb->flags |= POOL_BUCKET_PREALLOCATED;
            p->empty_stack = pb;
            p->empty_stack_size++;
            pb++;
        }

        p->data_buffer = calloc(prealloc_size, elt_size);
        /* FIXME better goto */
        if (p->data_buffer == NULL) {
            LOGE("alloc error");
            goto error;
        }
    }
    /* prealloc the buckets and requeue them to the alloc list */
    for (u32 = 0; u32 < prealloc_size; u32++) {
        if (size == 0) { /* unlimited */
            PoolBucket *pb = malloc(sizeof(PoolBucket));
            if (unlikely(pb == NULL)) {
                LOGE("alloc error");
                goto error;
            }
            memset(pb, 0, sizeof(PoolBucket));

            if (p->Alloc) {
                pb->data = p->Alloc();
            } else {
                pb->data = malloc(p->elt_size);
            }
            if (pb->data == NULL) {
                LOGE("alloc error");
                free(pb);
                goto error;
            }
            if (p->Init(pb->data, p->InitData) != 1) {
                LOGE("init error");
                if (p->Cleanup)
                    p->Cleanup(pb->data);
                if (p->Free)
                    p->Free(pb->data);
                else
                    free(pb->data);
                free(pb);
                goto error;
            }
            p->allocated++;

            pb->next = p->alloc_stack;
            p->alloc_stack = pb;
            p->alloc_stack_size++;
        } else {
            PoolBucket *pb = p->empty_stack;
            if (pb == NULL) {
                LOGE("alloc error");
                goto error;
            }

            pb->data = (char *)p->data_buffer + u32 * elt_size;
            if (p->Init(pb->data, p->InitData) != 1) {
                LOGE("init error");
                if (p->Cleanup)
                    p->Cleanup(pb->data);
                pb->data = NULL;
                goto error;
            }

            p->empty_stack = pb->next;
            p->empty_stack_size--;

            p->allocated++;

            pb->next = p->alloc_stack;
            p->alloc_stack = pb;
            p->alloc_stack_size++;
        }
    }

    return p;

error:
    if (p != NULL) {
        PoolFree(p);
    }
    return NULL;
}

void PoolFree(Pool *p)
{
    if (p == NULL)
        return;

    while (p->alloc_stack != NULL) {
        PoolBucket *pb = p->alloc_stack;
        p->alloc_stack = pb->next;
        if (p->Cleanup)
            p->Cleanup(pb->data);
        if (PoolDataPreAllocated(p, pb->data) == 0) {
            if (p->Free)
                p->Free(pb->data);
            else
                free(pb->data);
        }
        pb->data = NULL;
        if (!(pb->flags & POOL_BUCKET_PREALLOCATED)) {
            free(pb);
        }
    }

    while (p->empty_stack != NULL) {
        PoolBucket *pb = p->empty_stack;
        p->empty_stack = pb->next;
        if (pb->data!= NULL) {
            if (p->Cleanup)
                p->Cleanup(pb->data);
            if (PoolDataPreAllocated(p, pb->data) == 0) {
                if (p->Free)
                    p->Free(pb->data);
                else
                    free(pb->data);
            }
            pb->data = NULL;
        }
        if (!(pb->flags & POOL_BUCKET_PREALLOCATED)) {
            free(pb);
        }
    }

    if (p->pb_buffer)
        free(p->pb_buffer);
    if (p->data_buffer)
        free(p->data_buffer);
    free(p);
}

void PoolPrint(Pool *p)
{
    printf("\n----------- Hash Table Stats ------------\n");
    printf("Buckets:               %" PRIu32 "\n", p->empty_stack_size + p->alloc_stack_size);
    printf("-----------------------------------------\n");
}

void *PoolGet(Pool *p)
{
    PoolBucket *pb = p->alloc_stack;
    if (pb != NULL) {
        /* pull from the alloc list */
        p->alloc_stack = pb->next;
        p->alloc_stack_size--;

        /* put in the empty list */
        pb->next = p->empty_stack;
        p->empty_stack = pb;
        p->empty_stack_size++;
    } else {
        if (p->max_buckets == 0 || p->allocated < p->max_buckets) {
            void *pitem;
            LOGD("max_buckets %"PRIu32"", p->max_buckets);

            if (p->Alloc != NULL) {
                pitem = p->Alloc();
            } else {
                pitem = malloc(p->elt_size);
            }

            if (pitem != NULL) {
                if (p->Init(pitem, p->InitData) != 1) {
                    if (p->Cleanup)
                        p->Cleanup(pitem);
                    if (p->Free != NULL)
                        p->Free(pitem);
                    else
                        free(pitem);
                    return NULL;
                }

                p->allocated++;
                p->outstanding++;
#ifdef DEBUG
                if (p->outstanding > p->max_outstanding)
                    p->max_outstanding = p->outstanding;
#endif
            }

            return pitem;
        } else {
            return NULL;
        }
    }

    void *ptr = pb->data;
    pb->data = NULL;
    p->outstanding++;
#ifdef DEBUG
    if (p->outstanding > p->max_outstanding)
        p->max_outstanding = p->outstanding;
#endif
    return ptr;
}

void PoolReturn(Pool *p, void *data)
{
    PoolBucket *pb = p->empty_stack;

    LOGD("pb %p", pb);

    if (pb == NULL) {
        p->allocated--;
        p->outstanding--;
        if (p->Cleanup != NULL) {
            p->Cleanup(data);
        }
        if (PoolDataPreAllocated(p, data) == 0) {
            if (p->Free)
                p->Free(data);
            else
                free(data);
        }

        LOGD("tried to return data %p to the pool %p, but no more "
                   "buckets available. Just freeing the data.", data, p);
        return;
    }

    /* pull from the alloc list */
    p->empty_stack = pb->next;
    p->empty_stack_size--;

    /* put in the alloc list */
    pb->next = p->alloc_stack;
    p->alloc_stack = pb;
    p->alloc_stack_size++;

    pb->data = data;
    p->outstanding--;
    return;
}

void PoolPrintSaturation(Pool *p)
{
    LOGD("pool %p is using %"PRIu32" out of %"PRIu32" items (%02.1f%%), max %"PRIu32" (%02.1f%%): pool struct memory %"PRIu64".", p, p->outstanding, p->max_buckets, (float)(p->outstanding/(float)(p->max_buckets))*100, p->max_outstanding, (float)(p->max_outstanding/(float)(p->max_buckets))*100, (uint64_t)(p->max_buckets * sizeof(PoolBucket)));
}

/*
 * ONLY TESTS BELOW THIS COMMENT
 */

#ifdef UNITTESTS
static void *PoolTestAlloc(void)
{
    void *ptr = malloc(10);
    if (unlikely(ptr == NULL))
        return NULL;
    return ptr;
}
static int PoolTestInitArg(void *data, void *allocdata)
{
    size_t len = strlen((char *)allocdata) + 1;
    char *str = data;
    if (str != NULL)
        strlcpy(str,(char *)allocdata,len);
    return 1;
}

static void PoolTestFree(void *ptr)
{
    return;
}

static int PoolTestInit01 (void)
{
    Pool *p = PoolInit(10,5,10,PoolTestAlloc,NULL,NULL,PoolTestFree, NULL);
    if (p == NULL)
        return 0;

    PoolFree(p);
    return 1;
}

static int PoolTestInit02 (void)
{
    int retval = 0;

    Pool *p = PoolInit(10,5,10,PoolTestAlloc,NULL,NULL,PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    if (p->alloc_stack == NULL || p->empty_stack == NULL) {
        printf("list(s) not properly initialized (a:%p e:%p): ",
            p->alloc_stack, p->empty_stack);
        retval = 0;
        goto end;
    }

    if (p->Alloc != PoolTestAlloc) {
        printf("Alloc func ptr %p != %p: ",
            p->Alloc, PoolTestAlloc);
        retval = 0;
        goto end;
    }

    if (p->Cleanup != PoolTestFree) {
        printf("Free func ptr %p != %p: ",
            p->Cleanup, PoolTestFree);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}

static int PoolTestInit03 (void)
{
    int retval = 0;
    void *data = NULL;

    Pool *p = PoolInit(10,5,10,PoolTestAlloc,NULL,NULL,PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    data = PoolGet(p);
    if (data == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (p->alloc_stack_size != 4) {
        printf("p->alloc_stack_size 4 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    if (p->empty_stack_size != 6) {
        printf("p->empty_stack_size 6 != %" PRIu32 ": ", p->empty_stack_size);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}

static int PoolTestInit04 (void)
{
    int retval = 0;
    char *str = NULL;

    Pool *p = PoolInit(10,5,strlen("test") + 1,NULL, PoolTestInitArg,(void *)"test",PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    str = PoolGet(p);
    if (str == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (strcmp(str, "test") != 0) {
        printf("Memory not properly initialized: ");
        retval = 0;
        goto end;
    }

    if (p->alloc_stack_size != 4) {
        printf("p->alloc_stack_size 4 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    if (p->empty_stack_size != 6) {
        printf("p->empty_stack_size 6 != %" PRIu32 ": ", p->empty_stack_size);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}

static int PoolTestInit05 (void)
{
    int retval = 0;
    void *data = NULL;

    Pool *p = PoolInit(10,5,10,PoolTestAlloc,NULL, NULL,PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    data = PoolGet(p);
    if (data == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (p->alloc_stack_size != 4) {
        printf("p->alloc_stack_size 4 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    if (p->empty_stack_size != 6) {
        printf("p->empty_stack_size 6 != %" PRIu32 ": ", p->empty_stack_size);
        retval = 0;
        goto end;
    }

    PoolReturn(p, data);
    data = NULL;

    if (p->alloc_stack_size != 5) {
        printf("p->alloc_stack_size 5 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    if (p->empty_stack_size != 5) {
        printf("p->empty_stack_size 5 != %" PRIu32 ": ", p->empty_stack_size);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}

static int PoolTestInit06 (void)
{
    int retval = 0;
    void *data = NULL;
    void *data2 = NULL;

    Pool *p = PoolInit(1,0,10,PoolTestAlloc,NULL,NULL,PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    if (p->allocated != 0) {
        printf("p->allocated 0 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    data = PoolGet(p);
    if (data == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (p->allocated != 1) {
        printf("p->allocated 1 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    data2 = PoolGet(p);
    if (data2 != NULL) {
        printf("PoolGet returned %p, expected NULL: ", data2);
        retval = 0;
        goto end;
    }

    PoolReturn(p,data);
    data = NULL;

    if (p->allocated != 1) {
        printf("p->allocated 1 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    if (p->alloc_stack_size != 1) {
        printf("p->alloc_stack_size 1 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}

/** \test pool with unlimited size */
static int PoolTestInit07 (void)
{
    int retval = 0;
    void *data = NULL;
    void *data2 = NULL;

    Pool *p = PoolInit(0,1,10,PoolTestAlloc,NULL,NULL,PoolTestFree, NULL);
    if (p == NULL)
        goto end;

    if (p->max_buckets != 0) {
        printf("p->max_buckets 0 != %" PRIu32 ": ", p->max_buckets);
        retval = 0;
        goto end;
    }

    if (p->allocated != 1) {
        printf("p->allocated 1 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    data = PoolGet(p);
    if (data == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (p->allocated != 1) {
        printf("(2) p->allocated 1 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    data2 = PoolGet(p);
    if (data2 == NULL) {
        printf("PoolGet returned NULL: ");
        retval = 0;
        goto end;
    }

    if (p->allocated != 2) {
        printf("(3) p->allocated 2 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    PoolReturn(p,data);
    data = NULL;

    if (p->allocated != 2) {
        printf("(4) p->allocated 2 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    if (p->alloc_stack_size != 1) {
        printf("p->alloc_stack_size 1 != %" PRIu32 ": ", p->alloc_stack_size);
        retval = 0;
        goto end;
    }

    PoolReturn(p,data2);
    data2 = NULL;

    if (p->allocated != 1) {
        printf("(5) p->allocated 1 != %" PRIu32 ": ", p->allocated);
        retval = 0;
        goto end;
    }

    retval = 1;
end:
    if (p != NULL)
        PoolFree(p);
    return retval;
}
#endif /* UNITTESTS */

void PoolRegisterTests(void)
{
#ifdef UNITTESTS
    UtRegisterTest("PoolTestInit01", PoolTestInit01);
    UtRegisterTest("PoolTestInit02", PoolTestInit02);
    UtRegisterTest("PoolTestInit03", PoolTestInit03);
    UtRegisterTest("PoolTestInit04", PoolTestInit04);
    UtRegisterTest("PoolTestInit05", PoolTestInit05);
    UtRegisterTest("PoolTestInit06", PoolTestInit06);
    UtRegisterTest("PoolTestInit07", PoolTestInit07);

    PoolThreadRegisterTests();
#endif /* UNITTESTS */
}


/**
 * @}
 */
