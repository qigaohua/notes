#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


#include "../libconfig.h"
// #include "config_util.h"
#include "cJSON.h"

static char *read_file(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    if (!filename)
        return NULL;

    file = fopen(filename, "rb");
    if (file == NULL) {
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0) {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        goto cleanup;
    }

    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL) {
        goto cleanup;
    }

    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length) {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';

cleanup:
    if (file != NULL) {
        fclose(file);
    }
    return content;
}

static int js_load(struct config *c, const char *name)
{
    cJSON *json;
    char *buf = read_file(name);
    if (!buf) {
        // printf("read_file %s failed!\n", name);
        return 0;
    }
    json = cJSON_Parse(buf);
    if (!json) {
        printf("cJSON_Parse failed!\n");
        free(buf);
        return -1;
    }
    free(buf);
    c->priv = (void *)json;
    strncpy(c->path, name, sizeof(c->path));
    return 0;
}

// static int js_set_string(struct config *c, ...)
// {
//     cJSON *json = (cJSON *)c->priv;
//     struct int_charp type_list[TYPE_LIST_COUNT] = {0};
//     struct int_charp mix;
//     int cnt = 0;
//     int i;
//     va_list ap;

//     va_start(ap, c);
//     va_arg_type(ap, mix);
//     while (mix.type != TYPE_EMPTY) {//last argument must be NULL
//         if (cnt++ > TYPE_LIST_COUNT) {
//             fprintf(stderr, "Too many type entry.\n");
//             return -1;
//         }
//         // type_list = (struct int_charp *)realloc(type_list, cnt*sizeof(struct int_charp));
//         memcpy(&type_list[cnt-1], &mix, sizeof(mix));
//         va_arg_type(ap, mix);
//     }
//     va_end(ap);

//     for (i = 0; i < cnt-1; i++) {
//         switch (type_list[i].type) {
//         case TYPE_INT:
//             json = cJSON_GetArrayItem(json, type_list[i].ival-1);
//             break;
//         case TYPE_CHARP:
//             json = cJSON_GetObjectItem(json, type_list[i].cval);
//             break;
//         default:
//             break;
//         }
//     }
//     cJSON_AddItemToObject(json, type_list[cnt-2].cval, cJSON_CreateString(type_list[cnt-1].cval));
//     // free(type_list);
//     return 0;
// }




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
}

*/

static int js_set_value(struct config *c, ...)
{
    cJSON *json = (cJSON *)c->priv;
    cJSON *new = NULL;
    struct int_charp type_list[TYPE_LIST_COUNT] = {0};
    struct int_charp mix;
    int cnt = 0;
    int i = 0;


    va_list ap;
    va_start(ap, c);
    va_arg_type_2(ap, mix);
    while (mix.type != TYPE_EMPTY) {//last argument must be NULL
        if (cnt++ > TYPE_LIST_COUNT) {
            fprintf(stderr, "Too many type entry.\n");
            return -1;
        }
        memcpy(&type_list[cnt-1], &mix, sizeof(mix));
        va_arg_type_2(ap, mix);
    }
    va_end(ap);

    if (cnt < 2) {
        fprintf(stderr, "Too few args !!!\n");
        return -1;
    }

    /*
     * 是否有数组值需要设置
     * array_type = 1 代表设置的数组类型没有键值，如 [1,2,3,4]
     * array_type = 2 代表设置的数组类型有键值，如 ["val":1, "val":2, "val":3]
     */
    int array_type = 0;
    for (i = 0; i < cnt-2 && array_type == 0; i++) {
        switch (type_list[i].type) {
        case TYPE_INT:
            break;
        case TYPE_CHARP:
        {
            if (json) {
                char *p = strchr(type_list[i].cval, ':');
                if (p) {
                    // 出现数组值需要设置，退出循环
                    char *cval = strdup(type_list[i].cval);
                    p = strchr(cval, ':');
                    *p++ = '\0';
                    array_type = atoi(p);
                    if (array_type != 1 && array_type != 2) {
                        fprintf(stderr, "Error: invaild args !!!\n");
                        free(cval); return -1;
                    }
                    // 通过数组名称得到数组object
                    cJSON *array = cJSON_GetObjectItem(json, cval);
                    if (array) { // 数组object存在
                        json = array;
                    }
                    else { // 如果数组不存在，则创建一个数组添加到json中
                        // index = 1;
                        cJSON_AddArrayToObject(json, cval);
                        json = cJSON_GetObjectItem(json, cval);
                    }
                    free(cval);
                    continue;
                }

                new = cJSON_GetObjectItem(json, type_list[i].cval);
                if (new) {
                    json = new;
                    continue;
                }
                new = cJSON_CreateObject();
                cJSON_AddItemToObject(json, type_list[i].cval,  new);
                json = cJSON_GetObjectItem(json, type_list[i].cval);
            }
            else {
                // 第一次添加数据时，json是为null的
                json = cJSON_CreateObject();
                c->priv = (void *)json;
                new = cJSON_CreateObject();
                cJSON_AddItemToObject(json, type_list[i].cval,  new);
                json = cJSON_GetObjectItem(json, type_list[i].cval);
            }
            break;
        }
        default:
            break;
        }
    }

    /* 参数中有数组值需要设置 */
    if (array_type != 0) {
        int count = cnt - i;

        /* 设置没有键值的数组 */
        if (array_type == 1) {
            for (; count > 0; i+=1, count-=1) {
                if (type_list[i].type == TYPE_CHARP)
                    cJSON_AddItemToArray(json,cJSON_CreateString(type_list[i].cval));
                else
                    cJSON_AddItemToArray(json,cJSON_CreateNumber(type_list[i].ival));
            }
        }
        else if (array_type == 2) {
            /* 有键值的数组，后面参数成对出现 */
            if (count % 2 != 0) {
                fprintf(stderr, "Error: invaild args !!!\n");
                return -1;
            }
            cJSON *item = cJSON_CreateObject();
            for (; count > 0; i+=2, count-=2) {
                if (type_list[i+1].type == TYPE_CHARP)
                    cJSON_AddStringToObject(item,type_list[i].cval,
                            type_list[i+1].cval);
                else
                    cJSON_AddNumberToObject(item,type_list[i].cval,
                            type_list[i+1].ival);
            }
            cJSON_AddItemToArray(json, item);
        }
        else
            fprintf(stderr, "Error: invaild args !!!\n");

        return 0;
    }


    /* 设置非数组对象的值 */

    // json object 第一次添加数据
    if (!json && cnt == 2) {
        json = cJSON_CreateObject();
        c->priv = (void *)json;
    }

    if (type_list[cnt-1].type == TYPE_CHARP) {
        cJSON_AddItemToObject(json, type_list[cnt-2].cval,
                cJSON_CreateString(type_list[cnt-1].cval));
    }
    else if (type_list[cnt-1].type == TYPE_INT) {
            cJSON_AddItemToObject(json, type_list[cnt-2].cval,
                    cJSON_CreateNumber(type_list[cnt-1].ival));
    }
    else
    ;

    return 0;
}


static const char *js_get_string(struct config *c, ...)
{
    cJSON *json = (cJSON *)c->priv;
    // struct int_charp *type_list = NULL;
    struct int_charp type_list[TYPE_LIST_COUNT] = {0};
    struct int_charp mix;
    int cnt = 0;
    int i;
    va_list ap;

    va_start(ap, c);
    va_arg_type(ap, mix);
    while (mix.type != TYPE_EMPTY) {//last argument must be NULL
        if (cnt++ > TYPE_LIST_COUNT) {
            fprintf(stderr, "Too many type entry.\n");
            return NULL;
        }
        // type_list = (struct int_charp *)realloc(type_list, cnt*sizeof(struct int_charp));
        memcpy(&type_list[cnt-1], &mix, sizeof(mix));
        va_arg_type(ap, mix);
    }
    va_end(ap);

    for (i = 0; i < cnt; i++) {
        switch (type_list[i].type) {
        case TYPE_INT:
            json = cJSON_GetArrayItem(json, type_list[i].ival-1);
            break;
        case TYPE_CHARP:
            json = cJSON_GetObjectItem(json, type_list[i].cval);
            break;
        default:
            break;
        }
    }
    // free(type_list);
    return json->valuestring;
}

static int js_get_int(struct config *c, ...)
{
    cJSON *json = (cJSON *)c->priv;
    // struct int_charp *type_list = NULL;
    struct int_charp type_list[TYPE_LIST_COUNT] = {0};
    struct int_charp mix;
    int cnt = 0;
    int i;
    va_list ap;

    va_start(ap, c);
    va_arg_type(ap, mix);
    while (mix.type != TYPE_EMPTY) {//last argument must be NULL
        if (cnt++ > TYPE_LIST_COUNT) {
            fprintf(stderr, "Too many type entry.\n");
            return -1;
        }
        // type_list = (struct int_charp *)realloc(type_list, cnt*sizeof(struct int_charp));
        memcpy(&type_list[cnt-1], &mix, sizeof(mix));
        va_arg_type(ap, mix);
    }
    va_end(ap);

    for (i = 0; i < cnt; i++) {
        switch (type_list[i].type) {
        case TYPE_INT:
            json = cJSON_GetArrayItem(json, type_list[i].ival-1);
            break;
        case TYPE_CHARP:
            json = cJSON_GetObjectItem(json, type_list[i].cval);
            break;
        default:
            break;
        }
    }
    // free(type_list);
    return json->valueint;
}

static double js_get_double(struct config *c, ...)
{
    cJSON *json = (cJSON *)c->priv;
    // struct int_charp *type_list = NULL;
    struct int_charp type_list[TYPE_LIST_COUNT] = {0};
    struct int_charp mix;
    int cnt = 0;
    int i;
    va_list ap;

    va_start(ap, c);
    va_arg_type(ap, mix);
    while (mix.type != TYPE_EMPTY) {//last argument must be NULL
        if (cnt++ > TYPE_LIST_COUNT) {
            fprintf(stderr, "Too many type entry.\n");
            return -1;
        }
        // type_list = (struct int_charp *)realloc(type_list, cnt*sizeof(struct int_charp));
        memcpy(&type_list[cnt-1], &mix, sizeof(mix));
        va_arg_type(ap, mix);
    }
    va_end(ap);

    for (i = 0; i < cnt; i++) {
        switch (type_list[i].type) {
        case TYPE_INT:
            json = cJSON_GetArrayItem(json, type_list[i].ival-1);
            break;
        case TYPE_CHARP:
            json = cJSON_GetObjectItem(json, type_list[i].cval);
            break;
        default:
            break;
        }
    }
    // free(type_list);
    return json->valuedouble;
}

static int js_get_boolean(struct config *c, ...)
{
    cJSON *json = (cJSON *)c->priv;
    // struct int_charp *type_list = NULL;
    struct int_charp type_list[TYPE_LIST_COUNT] = {0};
    struct int_charp mix;
    int cnt = 0;
    int i;
    va_list ap;

    va_start(ap, c);
    va_arg_type(ap, mix);
    while (mix.type != TYPE_EMPTY) {//last argument must be NULL
        if (cnt++ > TYPE_LIST_COUNT) {
            fprintf(stderr, "Too many type entry.\n");
            return -1;
        }
        // type_list = (struct int_charp *)realloc(type_list, cnt*sizeof(struct int_charp));
        memcpy(&type_list[cnt-1], &mix, sizeof(mix));
        va_arg_type(ap, mix);
    }
    va_end(ap);

    for (i = 0; i < cnt; i++) {
        switch (type_list[i].type) {
        case TYPE_INT:
            json = cJSON_GetArrayItem(json, type_list[i].ival-1);
            break;
        case TYPE_CHARP:
            json = cJSON_GetObjectItem(json, type_list[i].cval);
            break;
        default:
            break;
        }
    }
    // free(type_list);

    if(!strcasecmp(json->valuestring, "true")) {
        return 1;
    } else {
        return 0;
    }
}

static void js_dump(struct config *c, FILE *f)
{
    cJSON *json = (cJSON *)c->priv;
    char *tmp = cJSON_Print(json);
    if (tmp) {
        printf("%s\n", tmp);
        free(tmp);
    }
}

static void js_unload(struct config *c)
{
    cJSON *json = (cJSON *)c->priv;
    if (json) {
        cJSON_Delete(json);
        json = NULL;
    }
}

struct config_ops json_ops = {
    js_load,
    js_set_value,
    js_get_string,
    js_get_int,
    js_get_double,
    js_get_boolean,
    NULL,
    NULL,
    js_dump,
    NULL,
    js_unload,
};
