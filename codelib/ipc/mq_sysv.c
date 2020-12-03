#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "ipc.h"

#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }


typedef struct mq_sysv_data {
    int msg_type;
    char buf[0];
} mq_msg_t;


/* note:
 *  mq_name must be nember string
 *  role 为了兼容，没有使用
 */
static int mq_sysv_open(const char *mq_name, int role)
{
    int msgid = -1;
    char *endptr;

    int val = strtol(mq_name, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
        logw("Call strtol() failed: %s", strerror(errno));
        return -1;
    }

    if (endptr == mq_name || *endptr != '\0') {
        logw("No digits were found, argument must number string, eg: \"123456\"");
        return -1;
    }

    key_t key = (key_t)val;

    msgid = msgget(key, IPC_CREAT | 0666);
    if (-1 == msgid) {
        logw("Call msgget() failed: %s", strerror(errno));
        return -1;
    }

    return msgid;
}


static int mq_sysv_close(int msgid)
{
    if (0 != msgctl(msgid, IPC_RMID, NULL)) {
        logw("Call msgctl() failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}


static ssize_t mq_sysv_send(int msgid, void *buf, size_t len)
{
    CHECK_ARGS(msgid < 0 || !buf || len <= 0, -1);
    mq_msg_t *msg = NULL;
    ssize_t slen = -1;

    msg = calloc(1, sizeof(mq_msg_t) + len);
    if (!msg) {
        logw("Call calloc() failed: %s", strerror(errno));
        goto err;
    }

    msg->msg_type = 1;  // 消息类型，类似优先级的作用, 默认1
    memcpy(msg->buf, buf, len);

    /* note: argument 3 is buf length, not mq_msg_t length */
    if (-1 == (slen = msgsnd(msgid, msg, len, IPC_NOWAIT))) {
        logw("Call msgsend() failed: %s", strerror(errno));
        goto err;
    }

    free(msg);
    return slen;

err:
    if (msg) free(msg);
    return -1;
}


static ssize_t mq_sysv_recv(int msgid, void *buf, size_t len)
{
    CHECK_ARGS(msgid < 0 || !buf || len <= 0, -1);
    mq_msg_t *msg = NULL;
    ssize_t rlen = -1;

    msg = calloc(1, sizeof(mq_msg_t) + len);
    if (!msg) {
        logw("Call calloc() failed: %s", strerror(errno));
        goto err;
    }

    /* note:
     * msgtype==0 表示获取队列中第一个可用的消息
     * msgtype>0  返回队列中消息类型为msgtype的第一个队列, 本源码中默认 msgtype=1
     * msgtype<0  返回队列中消息类型值小于或等于msgtype绝对值的消息，如果这种消息有若干个，则去类型值最小的消息
     */
    if (-1 ==  (rlen = msgrcv(msgid, msg, len, 0,  MSG_NOERROR | IPC_NOWAIT))) {
        logw("Call msgrcv() failed: %s", strerror(errno));
        goto err;
    }
    memcpy(buf, msg->buf, rlen);

    free(msg);
    return rlen;

err:
    if (msg) free(msg);
    return -1;

}


static int mq_sysv_unlink(const char *name)
{
    return 0;
}


struct ipc_ops mq_sysv_ops = {
    .open = mq_sysv_open,
    .close = mq_sysv_close,
    .read = mq_sysv_recv,
    .write = mq_sysv_send,
    .unlink = mq_sysv_unlink,
};






