#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"


void list_dump (const char *what, list_t *gl)
{
    int i;
    printf ("%s: #%p\t cnt %d\t size %d\t elems %p\n",
            what, gl, gl->gh_cnt, gl->gh_size, gl->elems);
    for (i = 0; i < gl->gh_cnt; i ++) {
        printf ("  #%d: %p at &%p\n", i, gl->elems[i], &gl->elems[i]);
    }
}


int list_grow (list_t* gl, int32_t size)
{
    if (gl->gh_flags & LIST_F_NOTGROWS)
        return -1;
    gl->gh_size += size;
    if (unlikely(gl->gh_size == 0))
        return 0;
    gl->elems =	(void **)gh_realloc(gl->elems,
            sizeof(*gl->elems) * gl->gh_size);

    return 0;
}


list_t* list_init (list_t *gl, int32_t size, func cb_free)
{
    check_param(gl, NULL);

    memset(gl, 0, sizeof(*gl));
    if (size > 0)
        list_grow(gl, size);

    gl->cb_free = cb_free;

    return gl;
}

list_t* list_new (int32_t size, func cb_free)
{
    list_t *gl = NULL;

    gl = (list_t *)gh_malloc(sizeof(*gl));

    list_init(gl, size, cb_free);
    gl->gh_flags |= LIST_F_ALLOCATED;

    return gl;
}


/*
 * 因为 gl->elems 是 (void **)（指针的指针）, 那么
 * 在qsort函数中返回比较的元素是 (void **)，
 * 所以我们需要一个跳板函数将 (void **) -> (void *)
 */
static __thread
int (*list_cmp_curr) (const void *, const void *);

static __inline
int list_cmp_trampoline (const void *_a, const void *_b) {
    const void *a = *(const void **)_a, *b = *(const void **)_b;

    return list_cmp_curr(a, b);
}

int32_t list_sort (list_t *gl,
        int32_t (*cmp)(const void *_a, const void *_b))
{
    check_param(gl, -1);
    check_param(cmp, -1);

    list_cmp_curr = cmp;
    qsort(gl->elems, gl->gh_cnt,
            sizeof(*gl->elems), list_cmp_trampoline);
    gl->gh_flags |= LIST_F_SORTED;

    return 0;
}

void*  list_elem (list_t *gl, int32_t i)
{
    if (likely(gl->gh_cnt > i))
        return (void *)gl->elems[i];
    return NULL;
}


void* list_find (list_t *gl, const void *elem,
        int32_t (*cmp)(const void *_a, const void *_b))
{
    int i;
    void *e;

    check_param(gl, NULL);
    check_param(elem, NULL);
    check_param(cmp, NULL);

    if (gl->gh_flags & LIST_F_SORTED) {
        void **g;

        list_cmp_curr = cmp;
        g = (void **)bsearch(&elem, gl->elems, gl->gh_cnt,
                sizeof(*gl->elems), list_cmp_trampoline);

        return g ? *g : NULL;
    }

    LIST_FOREACH (gl, i, e) {
        if (!cmp (elem, e))
            return e;
    }

    return NULL;
}


void list_free_cb (list_t *gl, void *ptr)
{
    if (gl->cb_free && ptr)
        gl->cb_free(ptr);
}


int32_t list_add_noduplicate (list_t *gl, void *elem,
        int32_t (*cmp)(const void *_a, const void *_b))
{
    check_param((gl && elem && cmp), -1);

    if (list_find(gl, elem, cmp))
        return 0;

    if (gl->gh_cnt + 1 >= gl->gh_size)
        list_grow(gl, gl->gh_size ? gl->gh_size * 2 : 16);

    gl->elems[gl->gh_cnt ++] = elem;
    gl->gh_flags &= ~LIST_F_SORTED;

    return 0;
}


int32_t list_add (list_t *gl, void *elem)
{
    check_param((gl || elem), -1);

    if (gl->gh_flags & LIST_F_UNIQUE)
        printf ("warning: the list not allow duplicates,"
                "you should use list_add_noduplicate\n");

    if (gl->gh_cnt + 1 >= gl->gh_size)
        list_grow(gl, gl->gh_size ? gl->gh_size * 2 : 16);

    gl->elems[gl->gh_cnt ++] = elem;
    gl->gh_flags &= ~LIST_F_UNIQUE;
    gl->gh_flags &= ~LIST_F_SORTED;

    return 0;
}


void* list_remove_index(list_t *gl, int32_t idx)
{
    void *e = NULL;

    if (likely(gl->gh_cnt > (idx+1))) {
        e = gl->elems[idx];
        memmove(&gl->elems[idx],
                &gl->elems[idx + 1],
                sizeof(*gl->elems) * (gl->gh_cnt - (idx+1)));
    }
    gl->gh_cnt --;

    return e;
}

void* list_remove (list_t *gl, void *elem)
{
    int32_t i;
    void *e;

    check_param(gl, NULL);
    check_param(elem, NULL);

    LIST_FOREACH(gl, i, e) {
        if (e == elem) {
            list_remove_index(gl, i);
            return e;
        }
    }

    return NULL;
}

void* list_remove_cmp (list_t *gl, void *elem,
        int32_t (*cmp)(void *_a, void *_b))
{
    int32_t i;
    void *e;
    // GH_UNUSED(gl);
    check_param(gl, NULL);
    check_param(elem, NULL);
    check_param(cmp, NULL);

    LIST_FOREACH(gl, i, e) {
        if (e == elem || !cmp(e, elem)) {
            list_remove_index(gl, i);
            return e;
        }
    }

    return NULL;
}


int32_t list_multi_remove_cmp (list_t *gl, void *elem,
        int32_t (*cmp)(const void *_a, const void *_b))
{
    int32_t i;
    void *e;
    int32_t cnt = 0;
    // GH_UNUSED(gl);
    check_param(gl, -1);
    check_param(elem, -1);
    check_param(cmp, -1);

    LIST_FOREACH(gl, i, e) {
        if (e == elem || !cmp(e, elem)) {
            list_remove_index(gl, i);
            list_free_cb(gl, e);
            cnt++;
        }
    }

    return cnt;
}


void __inline list_clear (list_t *gl)
{
    gl->gh_cnt = 0;
}


int32_t __inline list_cnt (list_t *gl)
{
    return gl->gh_cnt;
}

int32_t __inline list_empty (list_t *gl)
{
    return gl->gh_cnt == 0;
}


/* 默认copy 指针 */
void* list_onlycopy_ptr (void *s, void *mt)
{
    UNUSED(mt)
        return (void *)s;
}


int32_t list_copy_to (list_t *dst, list_t *src,
        void* (*copy)(void* _d, void *_s),
        void* match)
{
    int i;
    void *s, *d = NULL;

    check_param(dst, -1);
    check_param(src, -1);

    if (!copy)
        copy = list_onlycopy_ptr;

    LIST_FOREACH (src, i, s) {
        if (!s)
            break;
        d = copy(s, match);
        if (d)
            list_add(dst, d);
    }
    dst->cb_free = src->cb_free;
    dst->gh_flags = src->gh_flags;

    return 0;
}

void list_free (list_t *gl)
{
    int i;

    if (gl) {
        for (i = 0; i < gl->gh_cnt; i ++) {
            if (gl->cb_free)
                gl->cb_free(gl->elems[i]);
            else
                free(gl->elems[i]);

        }
        free(gl->elems);

        if (gl->gh_flags & LIST_F_ALLOCATED)
            free(gl);
    }
}


list_t* list_copy (list_t *src,
        void* (*copy)(void* _d, void *_s),
        void* match)
{
    list_t *dst = NULL;

    check_param(src, NULL);

    dst = list_new(src->gh_size, src->cb_free);

    if (list_copy_to(dst, src, copy, match) != 0) {
        list_free(dst);
        dst = NULL;
    }

    return dst;
}


#define TEST
#ifdef TEST

struct st {
    int a;
    double b;
    char *c;
};

int find_num = 0;
void test_free(void *ptr) {
    struct st *s = (struct st *)ptr;
    if (s) {
        if (s->c)
            free(s->c);
    }
    free(s);
}

int32_t test_cmp(void *_a, void *_b) {
    struct st *sa = (struct st *)_a;
    struct st *sb = (struct st *)_b;
    if (sa && sb) {
        return !(sa->a == sb->a);
    }
    return 1;
}

int32_t test_cmp2(const void *_a, const void *_b) {
    struct st *sa = (struct st *)_a;
    struct st *sb = (struct st *)_b;
    if (sa && sb) {
        find_num ++;
        return strcmp(sa->c, sb->c);
    }
    return 1;
}

void* test_copy(void* s, void* match)
{
    // GH_UNUSED(match)
    struct st *d;
    if (!s)
        return NULL;

    struct st *sp = (struct st *)s;

    d = (struct st *)malloc(sizeof(*d));
    memset(d, 0, sizeof(*d));

    if (!match) {
        d->a = sp->a;
        d->b = sp->b;
        d->c = (char *)malloc(strlen(sp->c) + 1);
        memset(d->c, 0, strlen(sp->c)+1);
        strncpy(d->c, sp->c, strlen(sp->c));
    } else {
        struct st *m = (struct st*) match;
        if (!strcmp(m->c, sp->c)) {
            d->a = sp->a;
            d->b = sp->b;
            d->c = (char *)malloc(strlen(sp->c) + 1);
            memset(d->c, 0, strlen(sp->c)+1);
            strncpy(d->c, sp->c, strlen(sp->c));
        } else {
            free (d);
            d = NULL;
        }
    }

    return (void*)d;
}


int main(int argc, char **argv)
{
    UNUSED(argc)
    UNUSED(argv)
    char c[][5] = {"fff", "bbb", "ggg", "eee", "ddd", "aaa", "ccc"};
    int i, idx;
    struct st *s;
#if 0
    list_t gl;

    //list_init(&gl, 16, free);
    list_init(&gl, 16, test_free);
#else
    list_t *gl;
    gl = list_new(16, test_free);
    gl->gh_flags |= LIST_F_UNIQUE;
#endif
    printf("======== test 添加与扩展   ==========\n");
    for (i = 0; i < 20; i ++) {
        s = (struct st *)malloc(sizeof(struct st));
        memset(s, 0, sizeof(*s));
        s->a = i;
        s->b = (double)(3.1415 * i);
        idx = i % (sizeof(c)/sizeof(c[0]));
        s->c = (char *)malloc(strlen(c[idx]) + 1);
        memset(s->c, 0, strlen(c[idx])+1);
        strncpy(s->c, c[idx], strlen(c[idx]));

        // list_add(gl, (void*)s);
        list_add_noduplicate(gl, (void*)s, test_cmp2);
    }
    for (i = 0; i < 20; i ++) {
        struct st *sp = (struct st *)list_elem(gl, i);
        if (NULL == sp) continue;

        printf("%d %f %s \n", sp->a, sp->b, sp->c);
    }
    printf("\n======== test delete ===============\n");
    /*test delete 1 */
    struct st *sd = (struct st *)list_elem(gl, 3); /*delete index = 3*/
    printf("delete:   %d %f %s \n", sd->a, sd->b, sd->c);
    list_remove(gl, sd);
    list_free_cb(gl, sd);

    /*test delete 2*/
    struct st sd2;
    struct st* sdp;
    sd2.a = 2;            /*delete s->a = 2*/
    sdp = (struct st *)list_remove_cmp(gl, (void *)&sd2, test_cmp);
    printf("delete:   %d %f %s \n", sdp->a, sdp->b, sdp->c);
    list_free_cb(gl, sdp);

    /*test delete 3*/
    struct st sd3;
    sd3.c = strdup("ggg");            /*delete all s->c = "ggg"*/
    int32_t cnt = list_multi_remove_cmp(gl, (void *)&sd3, test_cmp2);
    printf("delete num:  %d  \n", cnt);

    printf("\n======== test 排序  ==========\n");
    list_sort(gl, test_cmp2);

    for (i = 0; i < 20; i ++) {
        struct st *sp = (struct st *)list_elem(gl, i);
        if (NULL == sp) continue;

        printf("%d %f %s \n", sp->a, sp->b, sp->c);
    }

    printf("\n======== test find 1 : 用排序查找 ==========\n");
    find_num = 0;
    free(sd3.c);
    sd3.c = strdup("fff");
    struct st *sf = (struct st *)list_find (gl, &sd3, test_cmp2);
    if (sf)
        printf("find %d %f %s \n", sf->a, sf->b, sf->c);
    printf("find num: %d\n", find_num);

    printf("\n======== test find 2: 不用排序查找 =========\n");
    find_num = 0;
    gl->gh_flags &= ~LIST_F_SORTED;
    sf = (struct st *)list_find (gl, &sd3, test_cmp2);
    if (sf)
        printf("find %d %f %s \n", sf->a, sf->b, sf->c);
    printf("find num: %d\n", find_num);

    printf("\n======== test copy 1: 默认拷贝函数 =========\n");
    list_t *cp;
    cp = list_copy(gl, NULL, NULL);
    for (i = 0; i < 20; i ++) {
        struct st *sp = (struct st *)list_elem(cp, i);
        if (NULL == sp) continue;

        printf("%d %f %s \n", sp->a, sp->b, sp->c);
    }

    printf("\n==test copy 2: 根据自己定义数据结构编写拷贝函数 =====\n");
    list_t *cp2;
    cp2 = list_copy(gl, test_copy, NULL);
    for (i = 0; i < 20; i ++) {
        struct st *sp = (struct st *)list_elem(cp2, i);
        if (NULL == sp) continue;

        printf("%d %f %s \n", sp->a, sp->b, sp->c);
    }
    list_free(cp2);

    printf("\n==test copy 3: 复制相匹配的数据 =====\n");
    list_t *cp3;
    free(sd3.c);
    sd3.c = strdup("aaa");
    cp3 = list_copy(gl, test_copy, &sd3);
    for (i = 0; i < 20; i ++) {
        struct st *sp = (struct st *)list_elem(cp3, i);
        if (NULL == sp) continue;

        printf("%d %f %s \n", sp->a, sp->b, sp->c);
    }
    list_free(cp3);

    list_dump ("test", gl);

    list_free(gl);
    free(sd3.c);
    return 0;
}


#endif



