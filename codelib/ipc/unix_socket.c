#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <sys/un.h>


#include "ipc.h"


#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }

#define IPC_UNIX_SOCKET_PATH   "/tmp/ipc_domain_socket"
#define IPC_CLIENT    (0)
#define IPC_SERVER    (1)
// 1500 - 42 - 200
// 1518 - 18 - 20 -20 -12(tcp时间戳)
#define IPC_MTU   (1258)

/* note:
 *  prarm role: 0 for client, 1 for server
 */
static int unix_socket_open(const char *name, int role)
{
    CHECK_ARGS(!name, -1);
    int fd;
    char path[256] = {0};
    struct sockaddr_un saddr;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (-1 == fd) {
        logw("Call socket() failed: %s", strerror(errno));
        return -1;
    }

    snprintf(path, sizeof path, IPC_UNIX_SOCKET_PATH".%s", name);
    if (role == IPC_SERVER)
        unlink(path);

    memset(&saddr, 0, sizeof saddr);
    saddr.sun_family = PF_UNIX;
    strncpy(saddr.sun_path, path, sizeof saddr.sun_path);

    if (role == IPC_SERVER) {
        if (-1 == bind(fd, (struct sockaddr *)&saddr, sizeof(saddr))) {
            logw("Call bind() failed: %s", strerror(errno));
            goto err;
        }

        if (-1 == listen(fd, 16)) {
            logw("Call listen() failed: %s", strerror(errno));
            goto err;
        }
    }
    else if (role == IPC_CLIENT) {
        if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr))) {
            logw("Call connect() failed: %s", strerror(errno));
            goto err;
        }
    }
    else {
        logw("Error value of argument 2: 0 for client and 1 for server");
        return -1;
    }

    return fd;

err:
    if (fd > 0) close(fd);
    return -1;
}


static int unix_socket_close(int fd)
{
    if (-1 == close(fd)) {
        logw("Call close() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}


static int unix_socket_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    CHECK_ARGS(fd < 0 || !addr, -1);
    int cfd;

    cfd = accept(fd, (struct sockaddr *)&addr, addrlen);
    if (-1 == fd) {
        logw("Call accept() failed: %s", strerror(errno));
        return -1;
    }

    return cfd;
}


static ssize_t unix_socket_sendn(int fd, void *buf, size_t len)
{
    CHECK_ARGS(fd < 0 || !buf || len == 0, -1);
    ssize_t n;
    char *p = buf;
    size_t  left = len;

    while (left > 0) {
        n = send(fd, p, left > IPC_MTU ? IPC_MTU : left, 0);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            logw("Call send() failed: %s", strerror(errno));
            return -1;
        }
        else if (n == 0) {
            logw("Unix socket disconnected !!!")
            return 0;
        }
        else {
            left -= n;p += n;
            continue;
        }
    }

    return (len - left);
}


static ssize_t unix_socket_send(int fd, void *buf, size_t len)
{
    CHECK_ARGS(fd < 0 || !buf || len == 0, -1);
    ssize_t n;

    n = send(fd, buf, len, 0);
    if (n < 0) {
        logw("Call send() failed: %s", strerror(errno));
        return -1;
    }
    else if (n == 0) {
        logw("Unix socket disconnected !!!")
            return 0;
    }
    else {
        return n;
    }
}



static ssize_t unix_socket_recvn(int fd, void *buf, size_t len)
{
    CHECK_ARGS(fd < 0 || !buf || len == 0, -1);
    ssize_t n;
    char    *p = buf;
    size_t  left = len;

    while (left > 0) {
        n = recv(fd, p, left > IPC_MTU ? IPC_MTU : left, 0);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            logw("Call recv() failed: %s", strerror(errno));
            return -1;
        }
        else if (n == 0) {
            logw("Unix socket disconnected !!!")
            return 0;
        }
        else {
            left -= n;p += n;
            continue;
        }
    }

    return (len - left);
}


static ssize_t unix_socket_recv(int fd, void *buf, size_t len)
{
    CHECK_ARGS(fd < 0 || !buf || len == 0, -1);
    ssize_t n;

    n = recv(fd, buf, len, 0);
    if (n < 0) {
        logw("Call send() failed: %s", strerror(errno));
        return -1;
    }
    else if (n == 0) {
        logw("Unix socket disconnected !!!")
            return 0;
    }
    else {
        return n;
    }
}


static int unix_socket_setblock(int fd, int on)
{
    int flag;

    flag = fcntl(fd, F_GETFL);
    if (-1 == flag) {
        logw("Call fcntl() failed: %s", strerror(errno));
        return -1;
    }

    if (on)
        flag |= O_NONBLOCK;
    else
        flag &= ~O_NONBLOCK;

    if (-1 == fcntl(fd, F_SETFL, flag)) {
        logw("Call fcntl() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}


struct ipc_ops unix_socket_ops = {
    .open = unix_socket_open,
    .close = unix_socket_close,
    .read = unix_socket_recv,
    .readn = unix_socket_recvn,
    .write = unix_socket_send,
    .writen = unix_socket_sendn,
    .accept = unix_socket_accept,
    .setblock = unix_socket_setblock
};

