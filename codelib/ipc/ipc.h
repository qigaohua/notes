#ifndef _IPC_H
#define _IPC_H

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

struct ipc_ops {
    int     (*open)(const char *, int);
    int     (*close)(int);
    ssize_t (*read)(int, void *, size_t);
    ssize_t (*readn)(int, void *, size_t);
    ssize_t (*write)(int, void *, size_t);
    ssize_t (*writen)(int, void *, size_t);
    int     (*accept)(int, struct sockaddr *, socklen_t *);
    int     (*setblock)(int, int);
    int     (*unlink)(const char *);
};

#endif /* ifndef _IPC_H */
