/* Copyright (C) 2017-2100 All right reserved
 * For free
 *
 * @file lua_config.c
 * @brief
 * @author qigaohua, qigaohua168@163.com
 * @version 1.0.0
 * @date 2021-01-11
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <lua5.2/lua.h>

#include "../libconfig.h"
// #include "config_util.h"
#include "lua_c.h"


#ifndef MY_PRINT_INFO
#define MY_PRINT_INFO


#define B_RED(str)      "\033[1;31m" str "\033[0m"
#define B_GREEN(str)    "\033[1;32m" str "\033[0m"
#define B_YELLOW(str)   "\033[1;33m" str "\033[0m"

// #define _DEBUG
#if defined( _DEBUG )
#define ASSERT(x, msg) { \
    if (x) { \
        fprintf(stderr, B_RED("ASSERT at %s:%d : %s\r\n"), __FILE__, __LINE__, msg); \
        exit(1); \
    } \
}

#define DEBUG(code_segment) { code_segment }
#else
#define ASSERT(x, msg)
#define DEBUG(code_segment)
#endif

#define logerr(fmt, ...) {\
    fprintf(stderr, B_RED("%s:%d : "fmt"\r\n"), __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(1); \
}

#define logwarn(fmt, ...) {\
    fprintf(stderr, B_YELLOW("%s:%d  "fmt"\r\n"), __FILE__, __LINE__, ##__VA_ARGS__); \
}


#define logdebug(fmt, ...) {\
    fprintf(stdout, B_GREEN("%s:%d : "fmt"\r\n"), __FILE__, __LINE__, ##__VA_ARGS__); \
}

#endif


static int lua_config_load(struct config *c, const char *name)
{
    lua_State *L = NULL;

    ASSERT(!c || !name, "Param is null");

    L = lua_c_loadfile(name);
    if (!L) {
        logwarn("Call lua_c_loadfile failed !");
        return -1;
    }

    c->priv = (void *)L;
    strncpy(c->path, name, sizeof(c->path));
    return 0;
}


static const char *lua_config_get_string(struct config *c, ...)
{
    lua_State *L = (lua_State *)c->priv;
    const char *value;
    va_list ap;

    va_start(ap, c);
    value = lua_get_string(L, ap);
    if (!value)
        logwarn("lua_config_get_string failed !");

    return value;
}

static int lua_config_get_int(struct config *c, ...)
{
    lua_State *L = (lua_State *)c->priv;
    int value;
    va_list ap;

    va_start(ap, c);
    value = lua_get_integer(L, ap);
    if (value == LUA_INT_ERROR)
        logwarn("lua_config_get_int failed !");

    return value;
}

static double lua_config_get_double(struct config *c, ...)
{
    lua_State *L = (lua_State *)c->priv;
    double value;
    va_list ap;

    va_start(ap, c);
    value = lua_get_double(L, ap);
    if (value == LUA_DOUBLE_ERROR)
        logwarn("lua_config_get_double failed !");

    return value;
}

static int lua_config_get_boolean(struct config *c, ...)
{
    lua_State *L = (lua_State *)c->priv;
    int value;
    va_list ap;

    va_start(ap, c);
    value = lua_get_bool(L, ap);
    if (value < 0)
        logwarn("lua_config_get_boolean failed !");

    return value;
}

static void lua_config_dump(struct config *c, FILE *f)
{
}

static void lua_config_unload(struct config *c)
{
    lua_State *L = (lua_State *)c->priv;
    if (L)
        lua_close(L);
    L = NULL;
}

struct config_ops lua_ops = {
    lua_config_load,
    NULL,
    lua_config_get_string,
    lua_config_get_int,
    lua_config_get_double,
    lua_config_get_boolean,
    NULL,
    NULL,
    lua_config_dump,
    NULL,
    lua_config_unload,
};


