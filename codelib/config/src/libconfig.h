#ifndef LIBCONFIG_H
#define LIBCONFIG_H

#include <stdio.h>
#include <limits.h>
#if defined (__WIN32__) || defined (WIN32) || defined (_MSC_VER)
#include "libposix4win.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__WIN32__) || defined (WIN32) || defined (_MSC_VER)
#define PATH_MAX   4096
#endif
typedef struct config {
    struct config_ops *ops;
    char path[PATH_MAX];
    void *priv;
} config_t;

typedef struct config_ops {
    int (*load)(struct config *c, const char *name);
    int (*set_string)(struct config *c, ...);
    const char *(*get_string) (struct config *c, ...);
    int (*get_int)      (struct config *c, ...);
    double (*get_double)(struct config *c, ...);
    int (*get_boolean)  (struct config *c, ...);
    int (*get_length)   (struct config *c, ...);
    void (*del)(struct config *c, const char *key);
    void (*dump)(struct config *c, FILE *f);
    int (*save)(struct config *c);
    void (*unload)(struct config *c);
} config_ops_t;


#define TYPE_LIST_COUNT   32

#define TYPE_EMPTY   0
#define TYPE_INT     1
#define TYPE_CHARP   2

struct int_charp {
    int type;
    union {
        int ival;
        char *cval;
    };
};

/*
 * va_arg_type can get value from ap ignore type
 * firstly, try to match "char *", which must be pointer of memory in higher address
 * if the type is "int", the value force to "char *" will be the real value
 * because the int value is the index of table in conf file, and should be limited.
 * so we use MAX_CONF_ENTRY to divide the type of "int" or "char *"
 */
#define MAX_CONF_ENTRY 4096

#define va_arg_type(ap, mix)                        \
    do {                                            \
        char *__tmp = va_arg(ap, char *);           \
        if (!__tmp) {                               \
            mix.type = TYPE_EMPTY;                  \
            mix.cval = NULL;                        \
        } else if (__tmp < (char *)MAX_CONF_ENTRY) {\
            mix.type = TYPE_INT;                    \
            mix.ival = *(int *)&__tmp;              \
        } else {                                    \
            mix.type = TYPE_CHARP;                  \
            mix.cval = __tmp;                       \
        }                                           \
    } while (0)


// only for set value
// 这里使用长度来判断参数是整型还是字符串，一般情况下地址按十进制转化
// 字符串，其长度都会大于10，所以这里我们可以设置10亿以下的整型数
//（在不同的系统情况下，不确定一定成功）。
// 所以当我们要设置10亿以上整型数时，必须将其转换成字符串去设置。最好的是不要设
// 置整型数，把整型数都转化成字符串去设置。
#define va_arg_type_2(ap, mix)                                              \
    do {                                                                    \
        char *__tmp = va_arg(ap, char *);                                   \
        char __strtmp[128] = {0};                                           \
        snprintf(__strtmp, sizeof __strtmp, "%ld", (unsigned long)__tmp);   \
        int __len = strlen(__strtmp);                                       \
        if (__len <= 10) {                                                  \
            mix.type = TYPE_INT;                                            \
            mix.ival = *(int *)&__tmp;                                      \
        } else if ((__len > 10) && !strncmp(__tmp, "end", 3)) {             \
            mix.type = TYPE_EMPTY;                                          \
            mix.cval = NULL;                                                \
        } else {                                                            \
            mix.type = TYPE_CHARP;                                          \
            mix.cval = __tmp;                                               \
        }                                                                   \
    } while (0)





extern struct config *g_config;

struct config *conf_load(const char *name);
int conf_set(struct config *c, const char *key, const char *val);

/*
 * xxx = {
 *     yyy = {
 *         "aaa",
 *         "bbb",
 *     }
 * }
 * conf_get_type(c, "xxx", "yyy", 1) will get "aaa"
 * conf_get_type(c, "xxx", "yyy", 2) will get "bbb"
 * 0 or NULL will be recorgize end of args, must start array with 1
 */
#define conf_get_int(c, ...) g_config->ops->get_int(c, __VA_ARGS__, NULL)
#define conf_get_string(c, ...) g_config->ops->get_string(c, __VA_ARGS__, NULL)
#define conf_set_string(c, ...) g_config->ops->set_string(c, __VA_ARGS__, "end")
#define conf_get_double(c, ...) g_config->ops->get_double(c, __VA_ARGS__, NULL)
#define conf_get_boolean(c, ...) g_config->ops->get_boolean(c, __VA_ARGS__, NULL)
#define conf_get_length(c, ...) g_config->ops->get_length(c, __VA_ARGS__, NULL)

void conf_del(struct config *c, const char *key);
void conf_dump(struct config *c);
int conf_save(struct config *c);
void conf_dump_to_file(FILE *f, struct config *c);
void conf_unload(struct config *c);


#ifdef __cplusplus
}
#endif
#endif
