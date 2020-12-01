#ifndef __SKT_H__
#define __SKT_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C"  {
#endif

int skt_tcp_connect(const char *host, uint16_t port);
int skt_udp_connect(const char *host, uint16_t port);
static int skt_openfd_of_hostname(const char *hostname,
        const char *port, int type);
int skt_open_tcpfd(const char *host, uint16_t port);
int skt_open_udpfd(const char *host, uint16_t port);


void skt_close(int fd);
void skt_destory();
ssize_t skt_rio_readn(int fd, void *buf, size_t n);
ssize_t skt_rio_writen(int fd, void *buf, size_t n);
int skt_send(int fd, const void *buf, size_t len);
int skt_sendto(int fd, const char *ip, uint16_t port,
                const void *buf, size_t len);
int skt_recv(int fd, void *buf, size_t len);
int skt_send_sync_recv(int fd, const void *sbuf, size_t slen,
                void *rbuf, size_t rlen, int timeout);
int skt_recvfrom(int fd, uint32_t *ip, uint16_t *port, void *buf, size_t len);

int skt_get_tcp_info(int fd, struct tcp_info *tcpi);
int skt_get_remote_addr_by_fd(int fd, uint32_t ip, uint16_t port);
int skt_getaddr_by_fd(int fd, uint32_t ip, uint16_t port);
uint32_t skt_addr_pton(const char *ip);
int skt_addr_ntop(char *str, size_t size, uint32_t ip);
int skt_get_local_info(void);
int skt_set_noblk(int fd, int enable);
int skt_set_block(int fd);
int skt_set_nonblock(int fd);
int skt_set_reuse(int fd, int enable);
int skt_set_tcp_keepalive(int fd, int enable);
int skt_set_buflen(int fd, int size);
int get_mac_by_name(const char *device, unsigned char *buf);
int str_is_ip(const char *str, int str_len);
unsigned long ip_get_mask(unsigned long addr);

#ifdef __cplusplus
}
#endif

#endif /* ifndef  __SKT_H__ */
