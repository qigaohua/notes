#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if_ether.h>
#include <net/if.h>

#include "skt.h"

#define MAX_ADDR_STRING 64
#define MTU   (1500 - 42 - 200)
#define MAX_RETRY_CNT  5

#if 0
int open_clientfd(const char *hostname, const char *port)
{
    int sfd, s;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    hints.ai_protocol = 0;          // any protocol
    s = getaddrinfo(hostname, port, &hints, &listp);
    if (0 != s) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        return -1;
    }

    for(p = listp; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sfd)
            continue;
        if (connect(sfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(sfd);
    }

    freeaddrinfo(listp);

    if ( !p ) {
        fprintf(stderr, "Can't open a connection.\n");
        return -1;
    }

    return sfd;
}

int open_listenfd(const char *port)
#define LISTENQ  1024
{
    int sfd, s, optval = 1;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // any ip address
    //设置了AI_NUMERICSERV 标志并且该参数未设置为NULL，
    //那么该参数必须是一个指向10进制的端口号字符串
    hints.ai_flags = AI_NUMERICSERV;   // using port number
    hints.ai_protocol = 0;          // any protocol
    // 设置了AI_PASSIVE 且 第一个参数为NULL,
    // 返回的地址是通配符地址(wildcard address, IPv4时是INADDR_ANY,IPv6时是IN6ADDR_ANY_INIT)
    s = getaddrinfo(NULL, port, &hints, &listp);
    if (0 != s) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        return -1;
    }

    for(p = listp; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sfd)
            continue;

        if (-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                    (const void *)&optval, sizeof(int))) {
            fprintf(stderr, "setsockopt failed: %m.\n");
            close(sfd);return -1;
        }

        if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    freeaddrinfo(listp);

    if ( !p ) {
        fprintf(stderr, "Can't open a connection.\n");
        return -1;
    }

    if (-1 == listen(sfd, LISTENQ)) {
        fprintf(stderr, "listen failed: %m\n");
        close(sfd);
        return -1;
    }

    return sfd;
}
#endif

static int _skt_connect(const char *host, uint16_t port, int type)
{
    int sfd, ret;
    struct sockaddr_in si;

    if (inet_aton(host, &si.sin_addr) == 0) {
        fprintf(stderr, "%s: invalid ip addr.\n", host);
        return -1;
    }
    si.sin_family = AF_INET;
    si.sin_port = htons(port);

    sfd = socket(AF_INET, type, type == SOCK_STREAM ? IPPROTO_TCP :
            IPPROTO_UDP);
    if (sfd < 0) {
        fprintf(stderr, "socket failed: %m\n");
        return -1;
    }

    ret = connect(sfd, (struct sockaddr *)&si, sizeof si);
    if (ret < 0) {
        fprintf(stderr, "connect failed: %m\n");
        skt_close(sfd);
        return -1;
    }

    return sfd;
}


int skt_tcp_connect(const char *host, uint16_t port)
{
    if (!host || port > USHRT_MAX) {
        fprintf(stderr, "args is invaild.");
        return -1;
    }
    return _skt_connect(host, port, SOCK_STREAM);
}


int skt_udp_connect(const char *host, uint16_t port)
{
    if (!host || port > USHRT_MAX) {
        fprintf(stderr, "args is invaild.");
        return -1;
    }
    return _skt_connect(host, port, SOCK_DGRAM);
}



/**
 * @brief _skt_openfd_of_hostname 如果host是主机名，调用该函数
 *
 * @param hostname  主机名
 * @param port      端口号字符串格式
 * @param type      socket type, SOCK_STREAM or SOCK_DGRAM
 *
 * @return
 */
static int skt_openfd_of_hostname(const char *hostname, const char *port,
        int type)
{
    int sfd, s;
    struct addrinfo hints, *listp, *p;
    char port_str[8] = {0};

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = type; // SOCK_DGRAM or SOCK_STREAM

    //设置了AI_NUMERICSERV 标志并且该参数未设置为NULL，
    //那么该参数必须是一个指向10进制的端口号字符串
    hints.ai_flags = AI_NUMERICSERV;   // using port number
    hints.ai_protocol = 0;          // any protocol

    s = getaddrinfo(hostname, port, &hints, &listp);
    if (0 != s) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        return -1;
    }

    for(p = listp; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sfd)
            continue;

        skt_set_reuse(sfd, 1);
        if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(sfd);
    }

#ifdef __DEBUG__
    char ip[64] = {0};
    skt_addr_ntop(ip, 64, ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr);
    printf(">>>>> socket %d on %s:%u %s %s <<<<<\n", sfd,
                    ip,
                    ntohs(((struct sockaddr_in*)p->ai_addr)->sin_port),
                    p->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM",
                    p->ai_protocol == IPPROTO_TCP ? "IPPROTO_TCP" : "IPPROTO_UDP");
#endif

    freeaddrinfo(listp);

    if ( !p ) {
        fprintf(stderr, "Can't open a connection.\n");
        return -1;
    }

    return sfd;
}


#define LISTENQ 1024
int skt_open_tcpfd(const char *host, uint16_t port)
{
    int sfd = -1;
    struct sockaddr_in si;

    if ( !host || port > USHRT_MAX ) {
        fprintf(stderr, "args is invaild.\n");
        return -1;
    }

    if (inet_aton(host, &si.sin_addr) == 0) {
        char port_str[8] = {0};
        snprintf(port_str, sizeof port_str, "%u", port);
        sfd = skt_openfd_of_hostname(host, port_str, SOCK_STREAM);
    }
    else {
        si.sin_family = AF_INET;
        si.sin_port = htons(port);

        sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (-1 == sfd) {
            fprintf(stderr, "socket failed: %m\n");
            return -1;
        }

        skt_set_reuse(sfd, 1);
        if (bind(sfd, (struct sockaddr*)&si, sizeof si) != 0) {
            fprintf(stderr, "bind failed: %m\n");
            skt_close(sfd);
            return -1;
        }
    }

    if (sfd > 0 && -1 == listen(sfd, LISTENQ)) {
        fprintf(stderr, "listen failed: %m\n");
        close(sfd);
        return -1;
    }

    return sfd;
}


int skt_open_udpfd(const char *host, uint16_t port)
{
    if ( !host || port > USHRT_MAX )
        return -1;

    int sfd;
    struct sockaddr_in si;

    if (inet_aton(host, &si.sin_addr) == 0) {
        char port_str[8] = {0};
        snprintf(port_str, sizeof port_str, "%u", port);
        sfd = skt_openfd_of_hostname(host, port_str, SOCK_DGRAM);
    }
    else {
        si.sin_family = AF_INET;
        si.sin_port = htons(port);

        sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (-1 == sfd) {
            fprintf(stderr, "socket failed: %m\n");
            return -1;
        }

        skt_set_reuse(sfd, 1);
        if (bind(sfd, (struct sockaddr*)&si, sizeof si) != 0) {
            fprintf(stderr, "bind failed: %m\n");
            skt_close(sfd);
            return -1;
        }
    }

    return sfd;
}



/**
 * @brief skt_close
 *
 * @param fd
 */
void skt_close(int fd)
{
	#if defined (__linux__) || defined (__CYGWIN__)
    close(fd);
  #endif
  #if defined (__WIN32__) || defined (WIN32) || defined (_MSC_VER)
  	closesocket(fd);
  #endif
}


/**
 * @brief skt_destory
 */
void skt_destory()
{
	#if defined (__WIN32__) || defined (WIN32) || defined (_MSC_VER)
		WSACleanup();
  #endif
}


ssize_t skt_rio_readn(int fd, void *buf, size_t n)
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


ssize_t skt_rio_writen(int fd, void *buf, size_t n)
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

int skt_send(int fd, const void *buf, size_t len)
{
    ssize_t n;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = MTU;
    int cnt = 0;

    if (buf == NULL || len == 0) {//0 length packet no need send
        printf("%s paraments invalid!\n", __func__);
        return -1;
    }

    while (left > 0) {
        if (left < step)
            step = left;
        n = send(fd, p, step, 0);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else if (n == 0) {
            perror("send");
            return -1;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT) {
                printf("reach max retry count\n");
                break;
            }
            continue;
        }
        printf("send failed(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return (len - left);
}

int skt_sendto(int fd, const char *ip, uint16_t port,
                const void *buf, size_t len)
{
    ssize_t n;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = MTU;
    struct sockaddr_in sa;
    int cnt = 0;

    if (buf == NULL || len == 0) {
        printf("%s paraments invalid!\n", __func__);
        return -1;
    }
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = ip?inet_addr(ip):INADDR_ANY;
    sa.sin_port = htons(port);

    while (left > 0) {
        if (left < step)
            step = left;
        n = sendto(fd, p, step, 0, (struct sockaddr*)&sa, sizeof(sa));
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else if (n == 0) {
            perror("sendto");
            return -1;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT)
                break;
            continue;
        }
        perror("sendto");
        return -1;
    }
    return (len - left);
}


int skt_recv(int fd, void *buf, size_t len)
{
    int n;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = MTU;
    int cnt = 0;
    if (buf == NULL || len == 0) {
        printf("%s paraments invalid!\n", __func__);
        return -1;
    }
    while (left > 0) {
        if (left < step)
            step = left;
        n = recv(fd, p, step, 0);
        if (n > 0) {
            p += n;
            left -= n;
            //continue;
            break;
        } else if (n == 0) {
            //perror("recv");//peer connect closed, no need print
            return 0;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT)
                break;
            continue;
        }
        perror("recv");
        return -1;
    }
    return (len - left);
}

int skt_send_sync_recv(int fd, const void *sbuf, size_t slen,
                void *rbuf, size_t rlen, int timeout)
{
    skt_send(fd, sbuf, slen);
    skt_set_noblk(fd, 0);
    skt_recv(fd, rbuf, rlen);

    return 0;
}

int skt_recvfrom(int fd, uint32_t *ip, uint16_t *port, void *buf, size_t len)
{
    int n;
    char *p = (char *)buf;
    int cnt = 0;
    size_t left = len;
    size_t step = MTU;
    struct sockaddr_in si;
    socklen_t si_len = sizeof(si);

    memset(&si, 0, sizeof(si));

    while (left > 0) {
        if (left < step)
            step = left;
        n = recvfrom(fd, p, step, 0, (struct sockaddr *)&si, &si_len);
        if (n > 0) {
            p += n;
            left -= n;
            //continue;
            break;
        } else if (n == 0) {
            perror("recvfrom");
            return -1;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT)
                break;
            continue;
        }
        perror("recvfrom");
        return -1;
    }

    *ip = si.sin_addr.s_addr;
    *port = ntohs(si.sin_port);

    return (len - left);
}





#if defined (__linux__) || defined (__CYGWIN__)
/**
 * @brief skt_get_tcp_info 查看当前TCP信息
 *
 * @param fd
 * @param tcpi
 *
 * @return
 */
int skt_get_tcp_info(int fd, struct tcp_info *tcpi)
{
    socklen_t len = sizeof(*tcpi);
    return getsockopt(fd, SOL_TCP, TCP_INFO, tcpi, &len);
}
#endif

#if 0
#if defined (__linux__) || defined (__CYGWIN__)
int skt_get_local_list(skt_addr_list_t **al, int loopback)
{
#ifdef __ANDROID__
#else
    struct ifaddrs * ifs = NULL;
    struct ifaddrs * ifa = NULL;
    skt_addr_list_t *ap, *an;

    if (-1 == getifaddrs(&ifs)) {
        printf("getifaddrs: %s\n", strerror(errno));
        return -1;
    }

    ap = NULL;
    *al = NULL;
    for (ifa = ifs; ifa != NULL; ifa = ifa->ifa_next) {

        char saddr[MAX_ADDR_STRING] = "";
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        if (!(ifa->ifa_addr))
            continue;
        if (ifa ->ifa_addr->sa_family == AF_INET) {
            if (!inet_ntop(AF_INET,
                           &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                           saddr, INET_ADDRSTRLEN))
                continue;
            if (strstr(saddr,"169.254.") == saddr)
                continue;
            if (!strcmp(saddr,"0.0.0.0"))
                continue;
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            if (!inet_ntop(AF_INET6,
                           &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr,
                           saddr, INET6_ADDRSTRLEN))
                continue;
            if (strstr(saddr,"fe80") == saddr)
                continue;
            if (!strcmp(saddr,"::"))
                continue;
        } else {
            continue;
        }
        if ((ifa->ifa_flags & IFF_LOOPBACK) && !loopback)
            continue;

        an = (skt_addr_list_t *)calloc(sizeof(skt_addr_list_t), 1);
        an->addr.ip = ((struct sockaddr_in *) ifa->ifa_addr)->sin_addr.s_addr;
        an->addr.port = 0;
        an->next = NULL;
        if (*al == NULL) {
            *al = an;
            ap = *al;
        } else {
            ap->next = an;
            ap = ap->next;
        }
    }
    freeifaddrs(ifs);
#endif
    return 0;
}
#endif

#endif


/**
 * @brief skt_get_remote_addr_by_fd 根据描述符获取远程ip和端口
 *
 * @param fd
 * @param ip
 * @param port
 *
 * @return
 */
int skt_get_remote_addr_by_fd(int fd, uint32_t ip, uint16_t port)
{
    struct sockaddr_in si;
    socklen_t len = sizeof(si);

    if (-1 == getpeername(fd, (struct sockaddr *)&(si.sin_addr), &len)) {
        printf("getpeername: %s\n", strerror(errno));
        return -1;
    }
    ip = si.sin_addr.s_addr;
    port = ntohs(si.sin_port);

    return 0;
}



/**
 * @brief skt_getaddr_by_fd 根据描述符获取本地ip和端口
 *
 * @param fd
 * @param ip
 * @param port
 *
 * @return
 */
int skt_getaddr_by_fd(int fd, uint32_t ip, uint16_t port)
{
    struct sockaddr_in si;
    socklen_t len = sizeof(si);
    memset(&si, 0, len);

    if (-1 == getsockname(fd, (struct sockaddr *)&si, &len)) {
        printf("getsockname: %s\n", strerror(errno));
        return -1;
    }
    ip = si.sin_addr.s_addr;
    port = ntohs(si.sin_port);

    return 0;
}

/**
 * @brief skt_addr_pton 字符串ip转整数
 *
 * @param ip   要转化的ip
 *
 * @return     转化成功的整数ip， 失败返回-1
 */
uint32_t skt_addr_pton(const char *ip)
{
    struct in_addr ia;
    int ret;

    ret = inet_pton(AF_INET, ip, &ia);
    if (ret == -1) {
        printf("inet_pton: %s\n", strerror(errno));
        return -1;
    } else if (ret == 0) {
        printf("inet_pton not in presentation format\n");
        return -1;
    }
    return ia.s_addr;
}



/**
 * @brief skt_addr_ntop ip转字符串
 *
 * @param str   保存ip字符串内存
 * @param size  str 大小
 * @param ip    要转化的ip
 *
 * @return      sucess return 0
 */
int skt_addr_ntop(char *str, size_t size, uint32_t ip)
{
    struct in_addr ia;
    char tmp[MAX_ADDR_STRING];

    if ( !str ) {
        printf("invaild: str is null\n");
        return -1;
    }

    ia.s_addr = ip;
    if (NULL == inet_ntop(AF_INET, &ia, tmp, INET_ADDRSTRLEN)) {
        printf("inet_ntop: %s\n", strerror(errno));
        return -1;
    }
    strncpy(str, tmp, size);

    return 0;
}


#if defined (__linux__) || defined (__CYGWIN__)
/*
 * 获取网络接口名、mac地址、网络掩码、ip地址、广播地址
 */
int skt_get_local_info(void)
{
    int fd;
    int interfaceNum = 0;
    struct ifreq buf[16];
    struct ifconf ifc;
    struct ifreq ifrcopy;
    char mac[16] = {0};
    char ip[32] = {0};
    char broadAddr[32] = {0};
    char subnetMask[32] = {0};

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        close(fd);
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
        interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
        printf("interface num = %d\n", interfaceNum);
        while (interfaceNum-- > 0) {
            printf("\ndevice name: %s\n", buf[interfaceNum].ifr_name);

            //ignore the interface that not up or not runing
            ifrcopy = buf[interfaceNum];
            if (ioctl(fd, SIOCGIFFLAGS, &ifrcopy)) {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the mac of this interface
            if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum]))) {
                memset(mac, 0, sizeof(mac));
                snprintf(mac, sizeof(mac), "%02x%02x%02x%02x%02x%02x",
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[0],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[1],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[2],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[3],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[4],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[5]);
                printf("device mac: %s\n", mac);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the IP of this interface
            if (!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum])) {
                snprintf(ip, sizeof(ip), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr));
                printf("device ip: %s\n", ip);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the broad address of this interface
            if (!ioctl(fd, SIOCGIFBRDADDR, &buf[interfaceNum])) {
                snprintf(broadAddr, sizeof(broadAddr), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_broadaddr))->sin_addr));
                printf("device broadAddr: %s\n", broadAddr);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the subnet mask of this interface
            if (!ioctl(fd, SIOCGIFNETMASK, &buf[interfaceNum])) {
                snprintf(subnetMask, sizeof(subnetMask), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_netmask))->sin_addr));
                printf("device subnetMask: %s\n", subnetMask);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }
        }
    } else {
        printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
#endif


int skt_set_noblk(int fd, int enable)
{
#if defined (__linux__) || defined (__CYGWIN__)
    int flag;
    flag = fcntl(fd, F_GETFL);
    if (flag == -1) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
    if (enable) {
        flag |= O_NONBLOCK;
    } else {
        flag &= ~O_NONBLOCK;
    }
    if (-1 == fcntl(fd, F_SETFL, flag)) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}


int skt_set_block(int fd)
{
#if defined (__linux__) || defined (__CYGWIN__)
    int flag;
    flag = fcntl(fd, F_GETFL);
    if (flag == -1) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
    flag &= ~O_NONBLOCK;
    if (-1 == fcntl(fd, F_SETFL, flag)) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}


int skt_set_nonblock(int fd)
{
#if defined (__linux__) || defined (__CYGWIN__)
	int flag;
    flag = fcntl(fd, F_GETFL);
    if (flag == -1) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
    flag |= O_NONBLOCK;
    if (-1 == fcntl(fd, F_SETFL, flag)) {
        printf("fcntl: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}


/*
 * 一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用
 * SO_REUSEPORT支持多个进程或者线程绑定到同一端口，提高服务器程序的性能
 */
int skt_set_reuse(int fd, int enable)
{
    int on = !!enable;

#ifdef SO_REUSEPORT
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (unsigned char *)&on, sizeof(on))) {
        printf("setsockopt SO_REUSEPORT: %s\n", strerror(errno));
        return -1;
    }
#endif
#ifdef SO_REUSEADDR
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (unsigned char *)&on, sizeof(on))) {
        printf("setsockopt SO_REUSEADDR: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}



/**
 * @brief skt_set_tcp_keepalive SO_KEEPALIVE保持连接检测对方主机是否崩溃，避免（服务器）永远阻塞于TCP连接的输入
 *
 * @param fd
 * @param enable
 *
 * @return
 */
int skt_set_tcp_keepalive(int fd, int enable)
{
    int on = !!enable;

#ifdef SO_KEEPALIVE
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                         (const void*)&on, (socklen_t) sizeof(on))) {
        printf("setsockopt SO_KEEPALIVE: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}



/**
 * @brief skt_set_buflen 修改缓冲区的大小
 *
 * @param fd
 * @param size  新的缓存区大小
 *
 * @return
 */
int skt_set_buflen(int fd, int size)
{
    int sz;

    sz = size;
    while (sz > 0) {
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
                             (const void*) (&sz), (socklen_t) sizeof(sz))) {
            sz = sz / 2;
        } else {
            break;
        }
    }

    if (sz < 1) {
        printf("setsockopt SO_RCVBUF: %s\n", strerror(errno));
    }

    sz = size;
    while (sz > 0) {
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                             (const void*) (&sz), (socklen_t) sizeof(sz))) {
            sz = sz / 2;
        } else {
            break;
        }
    }

    if (sz < 1) {
        printf("setsockopt SO_SNDBUF: %s\n", strerror(errno));
    }
    return 0;
}


/*
 * 通过网络接口（如eth1）获取 mac地址
 * device: eth1
 * buf:  mac
 */
int get_mac_by_name(const char *device, unsigned char *buf)
{
	int sockfd, ret;
	struct ifreq req;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		return 1;
	}
	snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", device);
	ret = ioctl(sockfd, SIOCGIFHWADDR, &req);
	close(sockfd);
	if (ret == -1) {
		return 1;
	}
	memcpy(buf, req.ifr_hwaddr.sa_data, ETH_ALEN);
	return 0;
}


/*
 * 判断字符串是不是一个合法ip
 */
int str_is_ip(const char *str, int str_len)
{
    int i, count;
    unsigned int val;

    for (i = 0, count = 0, val = 0; i < str_len; i++) {
        if (str[i] > '0' && str[i] <= '9') {
            val = val * 10 + (str[i] - 0x30);
            if (val > 255) {
                return 0;
            }
        } else if (str[i] == '0') {
            if (val == 0) {
                if (i == str_len -1) {
                    break;
                } else if (str[i+1] != '.') {
                    return 0;
                }
            } else {
                val = val * 10 + (str[i] - 0x30);
                if (val > 255) {
                    return 0;
                }
            }
        } else if (str[i] == '.') {
            count++;
            if (count > 3) {
                return 0;
            }
            val = 0;
        } else if (str[i] == ':') {
            break;
        } else {
            return 0;
        }
    }
    if (count == 3) {
        return 1;
    }

    return 1;
}


/*
 * 根据ip地址获取网络掩码
 */
unsigned long ip_get_mask(unsigned long addr)
{
  	unsigned long dst;

  	if (addr == 0L)
  		return(0L);	/* special case */

  	dst = ntohl(addr);
  	if (IN_CLASSA(dst))
  		return(htonl(IN_CLASSA_NET));
  	if (IN_CLASSB(dst))
  		return(htonl(IN_CLASSB_NET));
  	if (IN_CLASSC(dst))
  		return(htonl(IN_CLASSC_NET));

  	return(0);
}
