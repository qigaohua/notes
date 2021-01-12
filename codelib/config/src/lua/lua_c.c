/* Copyright (C) 2017-2100 All right reserved
 * For free
 *
 * @file luac_r.c
 * @brief
 * @author qigaohua, qigaohua168@163.com
 * @version 1.0.0
 * @date 2021-01-08
 */

#include <stdio.h>
#include <stdlib.h>

#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>

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

/**
 * @brief luc_c_loadfile 载入和运行lua配置文件
 *
 * @param filename lua 文件名称
 *
 * @return L
 */
lua_State* lua_c_loadfile(const char *filename)
{
    lua_State *L = NULL;

    if (!filename) {
        logwarn("param `filename' is null !");
        return NULL;
    }

    /* 创建lua_State, 打开lua标准库 */
    L = luaL_newstate();
    if (!L)
        logerr("Call luaL_newstate() failed !");

    luaL_openlibs(L);

    /* 载入和运行lua配置文件 */
    // luaL_loadfile(L, filename);
    // lua_pcall(L, 0, LUA_MULTRET, 0);

    /* 或者直接调用lua_dofile来完成载入和运行 */
    int ret = luaL_dofile(L, filename);


    logdebug("load lua config OK.");
    return L;
}


void lua_c_close(lua_State *L)
{
    ASSERT(!L, "param `L' is null !!!");

    lua_close(L);
}



/**
 * @brief lua_c_getvalue 根据给的参数得到值，把值放到栈顶
 *
 * @param L
 * @param stack_top  放入最后栈顶值
 * @param ap    传入的参数
 *
 * @return  -1 失败 0 成功
 */
static int lua_c_getvalue(lua_State *L, int *stack_top, va_list ap)
{
    const char *value = NULL;
    char *tmp = NULL; // 为参数值
    *stack_top = 0;

    ASSERT(!L, "param `L' is null !!!");

    tmp = va_arg(ap, char *);
    if (!tmp) {
        logwarn("Invaild params !");
        return -1;
    }

    //index为0，表示把栈全清空
    lua_settop(L, 0);

    // 根配置入栈
    lua_getglobal(L, tmp);
    (*stack_top)--; // note

    // 不是table直接返回
    if (!lua_istable(L, *stack_top)) {
        DEBUG (
        logdebug("It isn't table, stack_top: %d", *stack_top);
        )
        return 0;
    }

    tmp = va_arg(ap, char *);
    if (!tmp) {
        logwarn("why ! you are not smart !!!");
        return -1;
    }

    /*
     * 判断是否是数字索引, 在本代码中设定索引值不得大于4096
     * note: 如果索引大于4096，会发生错误
     */
    if (*(unsigned long *)&tmp < 4096) {
        // 压入index值
        int index = *(unsigned long*)&tmp;
        lua_pushnumber(L, index);
    }
    else {
        // 压入key值
        lua_pushstring(L, tmp);
    }
    (*stack_top)--;

    while ((tmp = va_arg(ap, char *)) != NULL) {
        /* 表示上一个key对应的是table, 把table值会放在栈顶，同时上一个压入的元素名字被弹出 */
        lua_gettable(L, *stack_top);
        *stack_top = -1;

        lua_pushstring(L, tmp);
        (*stack_top)--;

    }

    /* 值会放在栈顶，同时刚才压入的元素名字被弹出 */
    lua_gettable(L, *stack_top);
    *stack_top = -1;

    return 0;
}


#define LUA_CONFIG

#ifdef LUA_CONFIG

// 获取整数值
int lua_get_integer(lua_State *L, va_list ap)
{
    int v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值

    ASSERT(!L, "param `L' is null !!!");

    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return -1;

    if (!lua_isnumber(L, stack_top)) {
        logwarn("Error: item isn't integer %d!", stack_top);
        return -1;
    }

    v = lua_tointeger(L, stack_top);

    return v;
}


// 获取浮点数
double lua_get_double(lua_State *L, va_list ap)
{
    double v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值

    ASSERT(!L, "param `L' is null !!!");

    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return -1.0;

    if (!lua_isnumber(L, stack_top)) {
        logwarn("Error: item isn't integer %d!", stack_top);
        return -1.0;
    }

    v = lua_tonumber(L, stack_top);

    return v;
}



int lua_get_bool(lua_State *L, va_list ap)
{
    int v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值

    ASSERT(!L, "param `L' is null !!!");

    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return -1;

    if (!lua_isboolean(L, stack_top)) {
        logwarn("Error: item isn't boolean %d!", stack_top);
        return -1;
    }

    v = lua_toboolean(L, stack_top);

    return v;
}


const char* lua_get_string(lua_State *L, va_list ap)
{
    const char *v;
    int stack_top = 0; // 栈顶值

    ASSERT(!L, "param `L' is null !!!");

    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return NULL;

    if (!lua_isstring(L, stack_top)) {
        logwarn("Error: item isn't string!");
        return NULL;
    }
    v = lua_tostring(L, stack_top);

    return v;
}

#else

// 获取整数值
int lua_get_integer(lua_State *L, ...)
{
    int v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值
    va_list ap;

    ASSERT(!L, "param `L' is null !!!");

    va_start(ap, L);
    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return LUA_INT_ERROR;

    if (!lua_isnumber(L, stack_top)) {
        logwarn("Error: item isn't integer %d!", stack_top);
        return LUA_INT_ERROR;
    }

    v = lua_tointeger(L, stack_top);

    return v;
}


// 获取浮点数
double lua_get_double(lua_State *L, ...)
{
    double v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值
    va_list ap;

    ASSERT(!L, "param `L' is null !!!");

    va_start(ap, L);
    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return LUA_DOUBLE_ERROR;

    if (!lua_isnumber(L, stack_top)) {
        logwarn("Error: item isn't integer %d!", stack_top);
        return LUA_DOUBLE_ERROR;
    }

    v = lua_tonumber(L, stack_top);

    return v;
}



int lua_get_bool(lua_State *L, ...)
{
    int v;  // 需要获取的值，返回
    int stack_top = 0; // 栈顶值
    va_list ap;

    ASSERT(!L, "param `L' is null !!!");

    va_start(ap, L);
    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return -1;

    if (!lua_isboolean(L, stack_top)) {
        logwarn("Error: item isn't boolean %d!", stack_top);
        return -1;
    }

    v = lua_toboolean(L, stack_top);

    return v;
}


const char* lua_get_string(lua_State *L, ...)
{
    const char *v;
    int stack_top = 0; // 栈顶值
    va_list ap;

    ASSERT(!L, "param `L' is null !!!");

    va_start(ap, L);
    if (0 != lua_c_getvalue(L, &stack_top, ap))
        return NULL;

    if (!lua_isstring(L, stack_top)) {
        logwarn("Error: item isn't string!");
        return NULL;
    }
    v = lua_tostring(L, stack_top);

    return v;
}

#endif


/**
 *  lua_next() 这个函数的工作过程是：
 *   1) 先从栈顶弹出一个 key
 *   2) 从栈指定位置的 table 里取下一对 key-value，先将 key 入栈再将 value 入栈
 *   3) 如果第 2 步成功则返回非 0 值，否则返回 0，并且不向栈中压入任何值

 *  第 2 步中从 table 里取出所谓“下一对 key-value”是相对于第 1 步中弹出的 key 的。
 *  table 里第一对 key-value 的前面没有数据，所以先用 lua_pushnil() 压入一个 nil
 *  充当初始 key。

 *  注意开始的时候先用 lua_gettop() 取了一下 table 在栈中的正索引（前面说过了，在
 *  进行这个 lua_next() 过程之前先将 table 入栈，所以栈大小就是 table 的正索引），
 *  后面的 lua_next() 过程中不断的有元素出入栈，所以使用正索引来定位 table 比较方便。

 *  到了 table 中已经没有 key-value 对时，lua_next() 先弹出最后一个 key，然后发现
 *  已经没有数据了会返回 0，while 循环结束。所以这个 lua_next() 过程结束以后
 *   table 就又位于栈顶了。
 */

/**
 * @brief lua_c_parse_table 打印table内容
 *
 * @param L
 * @param table_name 要打印表名称，可以为null, 但要确保栈索引-1处是一个表
 *
 * @return -1 for failed, 0 for ok
 */
int lua_c_parse_table(lua_State *L, const char *table_name)
{
    int key_type = -1;
    int value_type = -1;

    if (!L) {
        logwarn("Param `L' is null.");
        return -1;
    }

    if (table_name)
        lua_getglobal(L, table_name);

    if (!lua_istable(L, -1)) {
        logwarn("It not is a table .");
        return -1;
    }
    // 现在堆栈包含：-1 =>table


    /*
     * 这里直接设置-2最好，因为递归使用该函数，lua_gettop() 得到的应该是第一个
     * table的索引，对于嵌套表来说，有点不对。
     *
     *   lua_table3={
     *       lua_table4 = {
     *           lt4_width = 100,
     *           lt4_height = 200,
     *           lua_table5 = {
     *               test = "test ok",
     *               lua_table8 = {
     *                   l8_1 = 1234567,
     *                   l8_2 = "12345678"
     *               }
     *           }
     *       },
     *       lt3_name = "qgh168",
     *   }
     * 对于上个表来说：
     * 第一次调用该函数,运行到该处时栈内容:
     * -1 =>lua_table3
     * 第二次递归调用该函数,运行到该处时栈内容:
     * -1 => lua_table4; -2 => key; -3 => lua_table3
     * 第三次递归调用该函数,运行到该处时栈内容:
     * -1 ==> lua_table5; -2 ==>key  -3 => lua_table4; -4 => key; -5 => lua_table3
     * .....
     * 所以lua_pushnil(L);后，此次递归要打印的table栈索引一直是-2
     */
    // int index = lua_gettop(L);

    lua_pushnil(L);
    //现在堆栈包含 -1 => nil; -2 => table

    while(lua_next(L, -2)) {
        // 栈内容: -1 => value; -2 => key; -3 => table
        key_type = lua_type(L, -2);
        value_type = lua_type(L, -1);

        if (LUA_TSTRING == key_type) {
            fprintf(stdout, "%s: ", lua_tostring(L, -2));
        }
        else if (LUA_TNUMBER == key_type) {
            fprintf(stdout, "%ld: ", lua_tointeger(L, -2));
        }
        else {
            logwarn("Invaild key type !!!");
            lua_pop(L, 1);
            continue;
        }

        switch(value_type) {
            case LUA_TSTRING:
                fprintf(stdout, "%s\n", lua_tostring(L, -1));
                break;
            case LUA_TNUMBER:
                fprintf(stdout, "%f\n", lua_tonumber(L, -1));
                break;
            case LUA_TBOOLEAN:
                fprintf(stdout, "%s\n",
                        lua_toboolean(L, -1) == 1 ? "true" : "false");
                break;
            case LUA_TTABLE:
                fprintf(stdout, "\n");
                lua_c_parse_table(L, NULL);
                break;
            default:
                  logwarn("Invaild value type !!!");
        }

        lua_pop(L, 1); // 弹出 value，让 key 留在栈顶
        // 栈内容: -1 => key; -2 => table
    }
    // 现在堆栈包含：-1 =>table
}




/**************************************************************************************/


//for test

// #define TEST
#if defined(TEST)

#include "lua_c.h"

// 测试 va_arg 把 int 类型 转换到char* 类型
int test_va_list(const char *type, ...)
{
    char *tmp = NULL;
    va_list ap;

    va_start(ap, type);

    tmp = va_arg(ap, char *);
    printf("1 -> %s\n", tmp);
    printf("1 -> %ld\n", *(unsigned long *)&tmp);

    tmp = va_arg(ap, char *);
    printf("2 -> %ld\n", *(unsigned long *)&tmp);
    tmp = va_arg(ap, char *);
    printf("3 -> %ld\n", *(unsigned long *)&tmp);
    tmp = va_arg(ap, char *);
    printf("4 -> %ld\n", *(unsigned long *)&tmp);
    return 0;
}


int main(int argc, char *argv[])
{
#if 0
    test_va_list("test", "abc", 123, 456, 789);
    exit(0);
#endif

    lua_State *L;

    L = lua_c_loadfile("./example.lua");


    printf("******* test global *******\n");

    printf("width: %d\theight: %d\n", lua_c_get_integer(L, "width"),
                lua_c_get_integer(L, "height"));
    printf("name: %s\n", lua_c_get_string(L, "name"));
    printf("bool_enable: %d\n", lua_c_get_bool(L, "bool_enable"));
    printf("bool_disable: %d\n", lua_c_get_bool(L, "bool_disable"));
    printf("double_test: %f\n", lua_c_get_double(L, "double_test"));


    printf("\n******* test table *******\n");

    printf("a: %d\tb: %d\tc: %d\n", lua_c_get_integer(L, "lua_table1", "a"),
                lua_c_get_integer(L, "lua_table1", "b"),
                lua_c_get_integer(L, "lua_table1", "c"));
    printf("d: %s\te: %s\tf: %s\n", lua_c_get_string(L, "lua_table2", "d"),
                lua_c_get_string(L, "lua_table2", "e"),
                lua_c_get_string(L, "lua_table2", "f"));

    printf("lt3_name: %s\n", lua_c_get_string(L, "lua_table3", "lt3_name"));
    printf("l3l4_w: %d\tl3l4_h: %d\n",
                lua_c_get_integer(L, "lua_table3", "lua_table4", "lt4_width"),
                lua_c_get_integer(L, "lua_table3", "lua_table4", "lt4_height")
        );
    printf("l3l4l5_test: %s\n",
                lua_c_get_string(L, "lua_table3", "lua_table4",
                "lua_table5", "test"));

    printf("\n******* test index table *******\n");
    printf("1 -> %d\t2 -> %d\t3 -> %d\n",
                lua_c_get_integer(L, "lua_table6", 1),
                lua_c_get_integer(L, "lua_table6", 2),
                lua_c_get_integer(L, "lua_table6", 3));

    printf("l7[1].name: %s\tsex: %d\n",
                lua_c_get_string(L, "lua_table7", 1, "name"),
                lua_c_get_bool(L, "lua_table7", 1, "sex"));

    printf("l7[2]: %d\n", lua_c_get_integer(L, "lua_table7", 2));


    printf("\n******* test index table *******\n");
    lua_c_parse_table(L, "lua_table3");



    lua_c_close(L);
    return 0;
}


#endif

