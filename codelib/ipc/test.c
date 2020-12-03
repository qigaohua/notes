#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>


#include "ipc.h"

extern struct ipc_ops mq_posix_ops;
extern struct ipc_ops mq_sysv_ops;
extern struct ipc_ops unix_socket_ops;
extern struct ipc_ops shm_ops;

// #define MQ_SYSV_OP
// #define MQ_POSIX_OP
#define SHM_OP
//#define UNIX_SOCKET_OP

int client(struct ipc_ops *ops, const char *name, int role)
{
    int fd;
    char rbuf[1024] = {0};
    ssize_t rlen;

    fd = ops->open(name, role);
    if (fd < 0) {
        fprintf(stderr, "Call open failed\n");
        exit(1);
    }

    int i = 0;
    while(1) {
        rlen = ops->read(fd, rbuf, sizeof rbuf);
        if (rlen > 0)
            printf("read msg: %s\n", rbuf);
        if (++i > 15)
            break;
        sleep(3);
    }

#ifndef MQ_SYSV_OP
    ops->close(fd);
#endif

    return 0;
}


int server(struct ipc_ops *ops, const char *name, int role)
{
#define TEST_WRITE_STRING   "test ipc code"
    int fd;
    char wbuf[1024] = {0};
    ssize_t wlen;

    fd = ops->open(name, role);
    if (fd < 0) {
        fprintf(stderr, "Call open failed\n");
        exit(1);
    }
#if defined(UNIX_SOCKET_OP)
    struct sockaddr_un saddr;
    socklen_t addrlen = sizeof(saddr);
    int cfd = ops->accept(fd, (struct sockaddr *)&saddr, &addrlen);
    if (-1 == cfd)
        exit(1);
#endif

    int i = 0;
    while(1) {
        snprintf(wbuf, sizeof wbuf, TEST_WRITE_STRING": %d", i);
#ifndef UNIX_SOCKET_OP
        wlen = ops->write(fd, wbuf, strlen(wbuf)+1);
#else
        wlen = ops->write(cfd, wbuf, strlen(wbuf)+1);
#endif
        if (wlen >= 0)
            printf("write ok %d !!!\n", i);
        if (++i > 15)
            break;
        sleep(3);
    }
    sleep(5);

    ops->close(fd);
#if (defined MQ_POSIX_OP) || (defined SHM_OP)
    ops->unlink(name);
#endif

    return 0;
}


int main(int argc, char *argv[])
{
#if defined(MQ_POSIX_OP)
    struct ipc_ops *ops = &mq_posix_ops;
#elif defined(MQ_SYSV_OP)
    struct ipc_ops *ops = &mq_sysv_ops;
#elif defined(SHM_OP)
    struct ipc_ops *ops = &shm_ops;
#else
    struct ipc_ops *ops = &unix_socket_ops;
#endif

#if 1
    server(ops, "12345", 1);
#else
    client(ops, "12345", 0);
#endif

    return 0;
}

