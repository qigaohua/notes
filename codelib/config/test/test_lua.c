/* Copyright (C) 2017-2100 All right reserved
 * For free
 *
 * @file test_lua.c
 * @brief
 * @author qigaohua, qigaohua168@163.com
 * @version 1.0.0
 * @date 2021-01-08
 */


#include "../src/libconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>


static int lua_test(void)
{
    struct config *c = conf_load("example.lua");
    printf("lua_test\n");

    printf("******* test global *******\n");

    printf("width: %d\theight: %d\n", conf_get_int(c, "width"),
                conf_get_int(c, "height"));
    printf("name: %s\n", conf_get_string(c, "name"));
    printf("bool_enable: %d\n", conf_get_boolean(c, "bool_enable"));
    printf("bool_disable: %d\n", conf_get_boolean(c, "bool_disable"));
    printf("double_test: %f\n", conf_get_double(c, "double_test"));


    printf("\n******* test table *******\n");

    printf("a: %d\tb: %d\tc: %d\n", conf_get_int(c, "lua_table1", "a"),
                conf_get_int(c, "lua_table1", "b"),
                conf_get_int(c, "lua_table1", "c"));
    printf("d: %s\te: %s\tf: %s\n", conf_get_string(c, "lua_table2", "d"),
                conf_get_string(c, "lua_table2", "e"),
                conf_get_string(c, "lua_table2", "f"));

    printf("lt3_name: %s\n", conf_get_string(c, "lua_table3", "lt3_name"));
    printf("l3l4_w: %d\tl3l4_h: %d\n",
                conf_get_int(c, "lua_table3", "lua_table4", "lt4_width"),
                conf_get_int(c, "lua_table3", "lua_table4", "lt4_height")
        );
    printf("l3l4l5_test: %s\n",
                conf_get_string(c, "lua_table3", "lua_table4",
                "lua_table5", "test"));

    printf("\n******* test index table *******\n");
    printf("1 -> %d\t2 -> %d\t3 -> %d\n",
                conf_get_int(c, "lua_table6", 1),
                conf_get_int(c, "lua_table6", 2),
                conf_get_int(c, "lua_table6", 3));

    printf("l7[1].name: %s\tsex: %d\n",
                conf_get_string(c, "lua_table7", 1, "name"),
                conf_get_boolean(c, "lua_table7", 1, "sex"));

    printf("l7[2]: %d\n", conf_get_int(c, "lua_table7", 2));


    printf("\n******* test index table *******\n");
    conf_unload(c);

    return 0;
}



int main(int argc, char **argv)
{
    lua_test();

    return 0;
}
