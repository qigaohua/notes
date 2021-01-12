#include "../src/libconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

/*
{
    index1: value1,
    index2: {
        index3:value2,
        index4: {
            index5:value3,
        }
    }
	key:[
			{
				"nodeId" : 1,
                "key"    : "aaaaa"
				...
			}
			,{
				"nodeId" : 2,
				"key"    : "bbbbb"
			}
		]

    server: {
        service: [
            {ip: "127.0.0.1", port: "12345", time: "1天"},
            {ip: "127.0.0.2", port: "12346", time: "2天"},
            {ip: "127.0.0.3", port: "12347", time: "3天"},
        ]
    }
    test_array: [
        "item1",
        "item2",
        "item3",
        "item4",
        "item5",
    ]
}


 */
static int json_test(void)
{
    // struct config *conf = conf_load("json/all.json");
    // printf("json_test\n");
    // printf("id = %s\n", conf_get_string(conf, "test", "rgn", 1, "id"));
    // printf("port= %d\n", conf_get_int(conf, "test", "rgn", 1, "port"));
    // conf_set_string(conf, "test", "rgn", 1, "idd", "update");
    // conf_unload(conf);


    struct config *conf2 = conf_load(NULL);


    conf_set_string(conf2, "index1", "value1");
    conf_set_string(conf2, "index2", "index3", "value2");
    conf_set_string(conf2, "index2", "index4", "index5" ,"value3");

    /*
     * key:1 表示要设置数组值, 代表设置的数组类型没有键值，如 [1,2,3,4]
     * key:2 表示要设置数组值, 代表设置的数组类型有键值，如 ["val":1, "val":2, "val":3]
     */
    conf_set_string(conf2, "key:2", "node", 1, "key", "aaaaa");
    conf_set_string(conf2, "key:2", "node", 2, "key", "bbbbb");
    conf_set_string(conf2, "key:2", "node", 3, "key", "eeeee");

    conf_set_string(conf2, "server", "service:2", "ip", "127.0.0.1",
            "port", 12345, "time", "1天");
    conf_set_string(conf2, "server", "service:2", "ip", "127.0.0.2",
            "port", 12346, "time", "2天");
    conf_set_string(conf2, "server", "service:2", "ip", "127.0.0.3",
            "port", 12347, "time", "3天");
    conf_set_string(conf2, "test_array:1", "item1", "item2", "item3");
    conf_set_string(conf2, "test_array:1", "item4", "item5");
    conf_dump(conf2);

    printf("ip = %s\n", conf_get_string(conf2, "server", "service", 1, "ip"));
    printf("port = %d\n", conf_get_int(conf2, "server", "service", 1, "port"));
    printf("time = %s\n", conf_get_string(conf2, "server", "service", 1, "time"));
    conf_unload(conf2);

    return 0;
}

int main(int argc, char **argv)
{
    json_test();
    return 0;
}
