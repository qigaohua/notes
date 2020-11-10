#ifndef _GHLIST_H
#define _GHLIST_H
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>


// typedef  signed char       int8_t;
// typedef  unsigned char  uint8_t;
// typedef  short          int16_t;
// typedef  unsigned short uint16_t;
// typedef  int            int32_t;
// typedef  unsigned int   uint32_t;

typedef void (*func)(void*);


#ifndef likely
#define likely(x)   __builtin_expect((x),1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect((x),0)
#endif

#define UNUSED(val) (void)val;


#define logerr(fmt, ...) \
            printf("[%s][%d] "#fmt"\r\n", \
	                       __FILE__, __LINE__, ##__VA_ARGS__)

#define check_param(p, type) \
		do { \
		    if (!(p)) { \
			    logerr("Invaild parameter"); \
			    return (type); \
			} \
		} while(0)

#define LIST_FOREACH(gl, index, elem) \
		for (index = 0; (elem = list_elem(gl, index)); index ++)

typedef struct list_s {
	int32_t gh_size;
	int32_t gh_cnt;
	void **elems;
	func cb_free;
    uint32_t gh_flags;
#define LIST_F_ALLOCATED 0x0001  /* The rd_list_t is allocated, will be free on destroy*/
#define LIST_F_SORTED 0x0010     /* Sort*/
#define LIST_F_UNIQUE 0x0100     /* Don't allow duplicates*/
#define LIST_F_NOTGROWS 0x1000   /* Don't allow grow*/
} list_t, *list_p;


#define gh_free free
static __inline void* gh_realloc(void *ptr, size_t size)
{
    void* p = realloc(ptr, size);
	if (!p) {
		perror("realloc error");
		exit(0);
	}
	return p;
}

static __inline void* gh_malloc(size_t size)
{
    void* p = malloc(size);
	if (!p) {
		perror("malloc error");
		exit(0);
	}
	return p;
}



list_t*     list_init(list_t *gl, int32_t size, func cb_free);
int         list_grow (list_t* gl, int32_t size);
list_t*     list_new (int32_t size, func cb_free);
int32_t     list_sort (list_t *gl,
				      int32_t (*cmp)(const void *_a, const void *_b));
list_t*     list_new (int32_t size, func cb_free);
void*       list_elem (list_t *gl, int32_t i);
void*       list_find (list_t *gl, const void *elem,
				    int32_t (*cmp)(const void *_a, const void *_b));
void        list_free_cb (list_t *gl, void *ptr);
int32_t     list_add_noduplicate (list_t *gl, void *elem,
				 int32_t (*cmp)(const void *_a, const void *_b));
int32_t     list_add (list_t *gl, void *elem);
void*       list_remove_index(list_t *gl, int32_t idx);
void*       list_remove (list_t *gl, void *elem);
void*       list_remove_cmp (list_t *gl, void *elem,
				         int32_t (*cmp)(void *_a, void *_b));
int32_t     list_multi_remove_cmp (list_t *gl, void *elem,
				         int32_t (*cmp)(const void *_a, const void *_b));
void    __inline list_clear (list_t *gl);
int32_t __inline list_cnt (list_t *gl);
int32_t __inline list_empty (list_t *gl);
void*       list_onlycopy_ptr (void *s, void *mt);
int32_t     list_copy_to (list_t *dst, list_t *src,
				      void* (*copy)(void* _d, void *_s),
					  void* match);
void        list_free (list_t *gl);
list_t*     list_copy (list_t *src,
				      void* (*copy)(void* _d, void *_s),
					  void* match);

#endif
