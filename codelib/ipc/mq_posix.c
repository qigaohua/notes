#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <mqueue.h>

#include "ipc.h"

#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }

#define IPC_MQ_POSIX_PREFIX  "/ipc_wq_posix"

// default
#define IPC_MAX_MSG     10
#define IPC_MAX_MSGSIZE 1024
// 消息优先级
#define IPC_WQ_POSIX_PRIO 0


/*
 * role 为了兼容，没有使用
 */
static mqd_t mq_posix_open(const char *mq_name, int role)
{
    CHECK_ARGS(!mq_name, -1);
    mqd_t mqd;
    struct mq_attr attr;
    char name[256] = {0};

    attr.mq_flags = 0;
    attr.mq_maxmsg = IPC_MAX_MSG;
    attr.mq_msgsize = IPC_MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    snprintf(name, sizeof name, IPC_MQ_POSIX_PREFIX".%s", mq_name);
    // mqd = mq_open(name, O_RDWR | O_CREAT | O_NONBLOCK , 0666,
    //         &attr);
    mqd = mq_open(name, O_RDWR | O_CREAT , 0666,
            &attr);
    if (-1 == mqd) {
        logw("Call mq_open failed: %m");
        return -1;
    }

    return mqd;
}


static int mq_posix_close(mqd_t mqd)
{
    if (-1 == mq_close(mqd)) {
        logw("Call mq_close failed: %m");
        return -1;
    }

    return 0;
}


static int mq_posix_unlink(const char *mq_name)
{
    char name[256] = {0};

    snprintf(name, sizeof name, IPC_MQ_POSIX_PREFIX".%s", mq_name);
    if (-1 == mq_unlink(name)) {
        logw("Call mq_unlink failed: %m");
        return -1;
    }

    return 0;

}


static int mq_posix_setattr(mqd_t mqd, struct mq_attr *attr)
{
    return 0;
}


static int mq_posix_setblock(mqd_t mqd, int on)
{
    struct mq_attr attr;
    struct mq_attr newattr;

    if (-1 == mq_getattr(mqd, &attr))
    {
        logw("Call mq_getattr failed: %m");
        return -1;
    }

    if (on)
        newattr.mq_flags = O_NONBLOCK;
    else {
        newattr.mq_flags = 0;
    }

    newattr.mq_maxmsg = attr.mq_maxmsg;
    newattr.mq_msgsize = attr.mq_msgsize;
    newattr.mq_curmsgs = attr.mq_curmsgs;

    if (-1 == mq_setattr(mqd, &newattr, &attr))
    {
        logw("Call mq_setattr failed: %m");
        return -1;
    }

    return 0;
}


static ssize_t mq_posix_send(mqd_t mqd, void *buf, size_t len)
{
    if (-1 == mq_send(mqd, buf, len, IPC_WQ_POSIX_PRIO)) {
        logw("Call mq_send failed: %m");
        return -1;
    }

    return 0;
}


static ssize_t mq_posix_recv(mqd_t mqd, void *buf, size_t len)
{
    ssize_t ret;
    struct mq_attr attr;

    if (-1 == mq_getattr(mqd, &attr)) {
        logw("Call mq_getattr failed: %m");
        return -1;
    }

    ret = mq_receive(mqd, buf, attr.mq_msgsize, IPC_WQ_POSIX_PRIO);
    if (-1 == ret) {
        logw("Call mq_receive failed: %m");
    }

    return ret;
}


#if 0
结构体 sigevent
union sigval {          /* Data passed with notification */
 	int  sival_int;         /* Integer value */
    void   *sival_ptr;      /* Pointer value */
};

struct sigevent {
	int sigev_notify; /* Notification method */
    int sigev_signo;  /* Notification signal */
    union sigval sigev_value;  /* Data passed with notification */
    void (*sigev_notify_function) (union sigval);/* Function used for thread notification (SIGEV_THREAD) */
    void *sigev_notify_attributes;/* Attributes for notification thread (SIGEV_THREAD) */
    pid_t sigev_notify_thread_id; /* ID of thread to signal (SIGEV_THREAD_ID) */
};
sigev_notify参数:
    SIGEV_NONE：空的提醒，事件发生时不做任何事情
    SIGEV_SIGNAL：向进程发送sigev_signo中指定的信号，具体详细的状况参照上面的文档，这涉及到sigaction的使用
    SIGEV_THREAD：通知进程在一个新的线程中启动sigev_notify_function函数，函数的实参是sigev_value，系统API自动启动一个线程，我们不用显式启动。

综上可知，sigevent结构体实际上是为各类系统调用API提供了一个统一的处理结构，我们只需要对提供sigevent结构的API，提供一个具体赋值结构体参数即可。下面通过mq_notify来实际说明该问题
#endif


typedef void (*mq_notify_cb)(union sigval);
int mq_notify_update(mqd_t mqd, mq_notify_cb func, void *args)
{
    struct sigevent sigev;

    memset(&sigev, 0, sizeof sigev);
    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_notify_function = func;
    sigev.sigev_notify_attributes = NULL;
    sigev.sigev_value.sival_ptr = args;

    if (-1 == mq_notify(mqd, &sigev)) {
        logw("Call mq_notify() failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}



struct ipc_ops mq_posix_ops = {
    .open = mq_posix_open,
    .close = mq_posix_close,
    .read = mq_posix_recv,
    .write = mq_posix_send,
    .setblock = mq_posix_setblock,
    .unlink = mq_posix_unlink,
};



