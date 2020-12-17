/**
 * @file netlink.c
 * @brief netlink user code
 * @author qigaohua, qigaohua168@163.com
 * @version 0.1
 * @date 2020-12-17
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/netlink.h>


#include "ipc.h"


#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }

#define IPC_NETLINK_BUFF_SIZE  1024

static size_t nlmsg_seq ;


#if 0
// netlink 常用宏定义

#define NLMSG_ALIGNTO   4U

/* 宏NLMSG_ALIGN(len)用于得到不小于len且字节对齐的最小数值 */
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )

/* Netlink 头部长度 */
#define NLMSG_HDRLEN     ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

/* 计算消息数据len的真实消息长度（消息体 +　消息头）*/
#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)

/* 宏NLMSG_SPACE(len)返回不小于NLMSG_LENGTH(len)且字节对齐的最小数值 */
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))

/* 宏NLMSG_DATA(nlh)用于取得消息的数据部分的首地址，设置和读取消息数据部分时需要使用该宏 */
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))

/* 宏NLMSG_NEXT(nlh,len)用于得到下一个消息的首地址, 同时len 变为剩余消息的长度 */
#define NLMSG_NEXT(nlh,len)  ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))

/* 判断消息是否 >len */
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
               (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
               (nlh)->nlmsg_len <= (len))

/* NLMSG_PAYLOAD(nlh,len) 用于返回payload的长度*/
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#endif


// 参数无意义
static int netlink_open(__attribute__((unused)) const char *name,
                        __attribute__((unused)) int role)
{
    int fd;
    struct sockaddr_nl addr;

    /* NETLINK_USERSOCK: Reserved for user mode socket protocols  */
    fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    if (-1 == fd) {
        logw("Call socket() failed: %s", strerror(errno));
        return -1;
    }

    bzero(&addr, sizeof addr);
    /*
     * 其中nl_family固定为AF_NETLINK或PF_NETLINK
     * nl_pad为填充字段，设置为0
     * nl_pid端口的ID号, 一般设置为当前进程的PID
     * nl_groups用于指定多播组，每一个bit对应一个多播组，如果设置为0，表示不加入任何多播组
     */
    addr.nl_family = PF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0;
    addr.nl_pad = 0;
    if (-1 == bind(fd, (struct sockaddr *)&addr, sizeof addr)) {
        logw("Call bind() failed: %s", strerror(errno));
        goto err;
    }

    return fd;
err:
    if (fd > 0) close(fd);
    return -1;
}


static ssize_t netlink_send(int fd, void *buf, size_t len)
{
    ssize_t ret;
    struct sockaddr_nl daddr;
    struct nlmsghdr *nlh;

    bzero(&daddr, sizeof daddr);
    /*
     * 其中nl_family固定为AF_NETLINK或PF_NETLINK
     * nl_pad为填充字段，设置为0
     * nl_pid设置为0, 发往内核
     * nl_groups用于指定多播组，每一个bit对应一个多播组，如果设置为0，表示不加入任何多播组
     */
    daddr.nl_family = PF_NETLINK;
    daddr.nl_pad = 0;  // 填充0
    daddr.nl_pid = 0;  // 发往内核，为0
    daddr.nl_groups = 0;

    nlh = calloc(1, NLMSG_LENGTH(len+1));
    if (!nlh) {
        logw("Call calloc() failed: %s",strerror(errno));
        return -1;
    }
    bzero(nlh, NLMSG_LENGTH(len+1));
    /*
     * nlmsg_len需要填充为包含netlink消息头的整个netlink消息的长度
     * nlmsg_type：消息状态，定义了以下4种通用的消息类型
     *       NLMSG_NOOP：不执行任何动作，必须将该消息丢弃；
     *       NLMSG_ERROR：消息发生错误；
     *       NLMSG_DONE：标识分组消息的末尾；
     *       NLMSG_OVERRUN：缓冲区溢出，表示某些消息已经丢失。
     *       NLMSG_MIN_TYPEK：预留
     * nlmsg_flags: 消息标记，它们用以表示消息的类型
     * nlmsg_seq: 是用于跟踪消息的序号
     * nlmsg_pid: 发送端口的ID号，对于内核来说该值就是0，
     *            对于用户进程来说就是其socket所绑定的ID号(一般为pid)
     */
    nlh->nlmsg_pid = getpid(); //
    nlh->nlmsg_len = NLMSG_LENGTH(len+1);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = nlmsg_seq ++;
    memcpy(NLMSG_DATA(nlh), buf, len);

#if 0
    /* 发送单个个请求 */
    ret = sendto(fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr,
            sizeof(daddr));
    if (-1 == ret) {
        logw("Call sendto() failed: %s", strerror(errno));
        return -1;
    }
#else
    /* 使用struct iovec iov[]数组 和 sendmsg可以实现一次调用发送多个消息请求 */
    struct msghdr mhdr;
    struct iovec iov;

    /*
     * iov_base: iov_base指向数据包缓冲区，即参数buff
     * iov_len是buff的长度。
     * msghdr中允许一次传递多个buff，以数组的形式组织在msg_iov中，
     * msg_iovlen就记录数组的长度 （即有多少个buff）
     */
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    /*
     * msg_name： 数据的目的地址，网络包指向sockaddr_in, netlink则指向sockaddr_nl;
     * msg_namelen: msg_name 所代表的地址长度
     * msg_iov: 指向的是缓冲区数组
     * msg_iovlen: 缓冲区数组长度
     * msg_control: 辅助数据，控制信息(发送任何的控制信息)
     * msg_controllen: 辅助信息长度
     * msg_flags: 消息标识
     */
    bzero(&mhdr, sizeof mhdr);
    mhdr.msg_name = (void *)&daddr;
    mhdr.msg_namelen = sizeof daddr;
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;

    ret = sendmsg(fd, &mhdr, 0);
    if (-1 == ret) {
        logw("Call sendmsg() failed: %s", strerror(errno));
        free(nlh);return -1;
    }
#endif
    free(nlh);

    return ret;
}


static ssize_t netlink_recv(int fd, void *buf, size_t len)
{
    ssize_t ret;
    struct sockaddr_nl daddr;
    struct nlmsghdr *nlh;
    char rbuf[IPC_NETLINK_BUFF_SIZE] = {0};

    bzero(&daddr, sizeof daddr);
    nlh = (struct nlmsghdr *)rbuf;

#if 0
    socklen_t socklen = sizeof daddr;
    ret = recvfrom(fd, nlh, IPC_NETLINK_BUFF_SIZE, 0, (struct sockaddr *)&daddr,
            &socklen);
    if (-1 == ret) {
        logw("Call recvfrom() failed: %s", strerror(errno));
        return -1;
    }
    len = nlh->nlmsg_len - NLMSG_HDRLEN;
    memcpy(buf, NLMSG_DATA(nlh), len);
#else
    struct iovec iov;
    struct msghdr mhdr;

    iov.iov_base = nlh;
    iov.iov_len = IPC_NETLINK_BUFF_SIZE;

    bzero(&mhdr, sizeof mhdr);
    mhdr.msg_name = (void *)&daddr;
    mhdr.msg_namelen = sizeof daddr;
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;

    ret = recvmsg(fd, &mhdr, 0);
    if (-1 == ret) {
        logw("Call recvmsg() failed: %s", strerror(errno));
        return -1;
    }

    // len = len > (nlh->nlmsg_len - NLMSG_HDRLEN) ? len : (nlh->nlmsg_len -
    //         NLMSG_HDRLEN);
    len = nlh->nlmsg_len - NLMSG_HDRLEN;
    memcpy(buf, NLMSG_DATA(nlh), len);
#endif

    return ret;
}


static int netlink_close(int fd)
{
    if (-1 == close(fd)) {
        logw("Call close() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}



struct ipc_ops netlink_ops = {
    .open = netlink_open,
    .read = netlink_recv,
    .write = netlink_send,
    .close = netlink_close,
};


