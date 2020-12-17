#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "ipc.h"


/*
 * 共享内存：
 *  优点：共享内存是IPC通信当中传输速度最快的通信方式没有之一
 *  缺点：共享内存并未提供同步机制, 需用信号量来实现对共享内存同步访问控制
 */




#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }

#define IPC_SHARM_DEFAULT_SIZE   1024
#define IPC_SHARM_SIZE_MAX       4


typedef struct ipc_shm {
    int  shmid;
    void *shm_addr;
    char shm_name[128];
} ipc_shm_t;

// 一个进程最多只允许4个不同的共享内存
static ipc_shm_t shm[IPC_SHARM_SIZE_MAX];


/* note:
 *  shm_name 为了兼容，必须使用数字字符串
 *  role 为了兼容，没有使用
 */
static int shm_open(const char *shm_name,
                    __attribute__((unused)) int role)
{
    CHECK_ARGS(!shm_name, -1);
    int shmid = -1;
    char *endptr;

    int val = strtol(shm_name, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
        logw("Call strtol() failed: %s", strerror(errno));
        return -1;
    }
    if (endptr == shm_name || *endptr != '\0') {
        logw("No digits were found, argument must number string, eg: \"123456\"");
        return -1;
    }

    key_t key = (key_t)val;
    /*
     *  arg 1: 程序需要提供一个参数key，它为共享内存段提供一个外部名.（每个IPC对
     *         象都与一个键 即key相关联，然后此键再由内核变换为标识符）.还有一个
     *         特殊的键值IPC_PRIVATE, 它用于创建一个只属于该创建进程的新共享内存，通常不会用到；
     *  arg 2: 以字节为单位指定需要共享的内存容量.
     *  arg 3: 包含9个比特的权限标志，它们的作用与创建文件时使用的mode标志是一样
     *         IPC_CREAT定义的一个特殊比特位，同时必须和权限标志按位或才能创建一
     *         个新的共享内存段
     */
    shmid = shmget(key, IPC_SHARM_DEFAULT_SIZE, IPC_CREAT | 0666);
    if (-1 == shmid) {
        logw("Call msgget() failed: %s", strerror(errno));
        return -1;
    }

    int i = 0;
    while (i < IPC_SHARM_SIZE_MAX && shm[i++].shmid != 0);
    if (i > IPC_SHARM_SIZE_MAX) {
        logw("Sharm num can't max %d", IPC_SHARM_SIZE_MAX);
        return -1;
    }
    shm[i-1].shmid = shmid;
    strncpy(shm[i-1].shm_name, shm_name, sizeof shm[0].shm_name);

    return shmid;
}


static ssize_t shm_send(int shmid, void *buf, size_t len)
{
    CHECK_ARGS(shmid < 0 || !buf || len == 0, -1);

    /*
     * arg 2: 指定共享内存连接到当前进程中的地址位置，
     *        通常为0，表示让系统来选择共享内存的地址
     * arg 3: 是一组标志位，通常为0。
     *        它还可取:SHM_RND,用以决定是否将当前共享内存段连接到指定的shmaddr
     *        上,该参数和shm_addr联合使用;
     *        SHM_RDONLY单独使用则是指让它使连接的内存段只读，否则以读写方式连接此内存段
     */
    int i = 0;
    while (i < IPC_SHARM_SIZE_MAX && shm[i++].shmid != shmid);
    if (i > IPC_SHARM_SIZE_MAX) {
        logw("Argument 1 invalid");
        return -1;
    }

    void *addr = shm[i-1].shm_addr;
    if (!addr) {
        addr = shmat(shmid, NULL, 0);
        if ((void *)-1 == addr) {
            addr = NULL;
            logw("Call shmat() failed: %s", strerror(errno));
            return -1;
        }
        shm[i-1].shm_addr = addr;
    }
    memcpy(addr, buf, len);

    return len;
}


static ssize_t shm_recv(int shmid, void *buf, size_t len)
{
    CHECK_ARGS(shmid < 0 || !buf || len == 0, -1);

    int i = 0;
    while (i < IPC_SHARM_SIZE_MAX && shm[i++].shmid != shmid);
    if (i > IPC_SHARM_SIZE_MAX) {
        logw("Argument 1 invalid");
        return -1;
    }

    void *addr = shm[i-1].shm_addr;
    if (!addr) {
        addr = shmat(shmid, NULL, 0);
        if ((void *)-1 == addr) {
            addr = NULL;
            logw("Call shmat() failed: %s", strerror(errno));
            return -1;
        }
        shm[i-1].shm_addr = addr;
    }
    memcpy(buf, addr, len);

    return len;
}


/*
 * 仅仅是共享内存分离但并未删除它，其标识符及其相关数据结构都在,
 * 只是使得该共享内存对当前进程不再可用。
 */
static int shm_close(int shmid)
{
    int i = 0;
    while (i < IPC_SHARM_SIZE_MAX && shm[i++].shmid != shmid);
    if (i > IPC_SHARM_SIZE_MAX) {
        logw("Argument 1 invalid");
        return -1;
    }
    void *addr = shm[i-1].shm_addr;

    if (!addr) {
        logw("Call shm_close() failed, shm_addr is null.");
        return -1;
    }
    if (-1 == shmdt(addr)) {
        logw("Call shmat() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}


/*
 * 删除共享内存段, 使用时要记得调用它，否则共享内存会一直存在
 */
static int shm_unlink(const char *shm_name)
{
    CHECK_ARGS(!shm_name, -1);

    int i = 0;
    while (i < IPC_SHARM_SIZE_MAX && 0 != strncmp(shm[i++].shm_name, shm_name,
                strlen(shm[i].shm_name)));
    if (i > IPC_SHARM_SIZE_MAX) {
        logw("Argument 1 invalid");
        return -1;
    }

    if (-1 == shmctl(shm[i-1].shmid, IPC_RMID, NULL)) {
        logw("Call shmctl() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}



struct ipc_ops shm_ops = {
    .open = shm_open,
    .close = shm_close,
    .read = shm_recv,
    .write = shm_send,
    .unlink = shm_unlink,
};
