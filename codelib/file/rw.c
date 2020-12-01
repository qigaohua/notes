#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>



/**
 * @brief rio_readn 一次性读n个字节
 *
 * @param fd
 * @param buf
 * @param n
 *
 * @return
 */
ssize_t rio_readn(int fd, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *ptr = (char *)buf;

    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return (-1);   // error
        }
        else if (nread == 0)   // EOF
            break;

        ptr += nread;
        nleft -= nread;
    }

    return (n - nleft);
}



/**
 * @brief rio_writen 一次性写n个字节
 *
 * @param fd
 * @param buf
 * @param n
 *
 * @return
 */
ssize_t rio_writen(int fd, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwrite;

    char *ptr = (char *)buf;

    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwrite = 0;
            else
                return (-1); // error
        }

        ptr += nwrite;
        nleft -= nwrite;
    }

    return n;
}


/*
 * 带有缓冲区的RIO函数。
 * 缓冲区存在的目的是为了减少因多次调用系统级IO函数，陷入内核态而带来的额外开销
 */
#define RIO_BUFSIZE 4096
typedef struct {
    int rio_fd;                //与内部缓冲区关联的描述符
    size_t rio_cnt;            //缓冲区中剩下的字节数
    char *rio_bufptr;          //指向缓冲区中下一个未读的字节
    char rio_buf[RIO_BUFSIZE];
} rio_t;


void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  //缓冲区为空，调用read填充
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
                sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* Interrupted by sig handler return */
                return -1;

        }
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}


ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;          /* errno set by read() */
        else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        bufp += nread;
    }

    return (n - nleft);         /* return >= 0 */
}

// 读一行
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    size_t  n;
    ssize_t rc;
    char c, *bufp = (char *)usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;

            }
        } else if (rc == 0) {
            if (n == 1)
                return 0; //第一次读取就到了EOF
            else
                break;    //读了一些数据后遇到EOF

        } else
            return -1;    /* Error */

    }
    *bufp = 0;

    return n-1;
}



