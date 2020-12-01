#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>


#include "skt.h"


int main(int argc, char *argv[])
{
    int sfd, confd;
    char rbuf[1024] = {0};
    size_t len;
    struct sockaddr_in cli;
    socklen_t clilen;


#if 0
    sfd = skt_open_tcpfd("localhost", 8888);
    if (sfd < 0) {
        printf("error\n");
        return -1;
    }

    confd = accept(sfd, (struct sockaddr *)&cli, &clilen);
    if (confd < 0) {
        skt_close(sfd);
        return -1;
    }

    len = read(confd, rbuf, sizeof rbuf);
    if (len > 0)
        printf ("read>> %s\n", rbuf);
    else
        printf("read error\n");

    skt_close(sfd);
#endif

    sfd = skt_open_udpfd("0.0.0.0", 8888);

    uint32_t ip;
    uint16_t port;
    len = skt_recvfrom(sfd, &ip, &port, rbuf, sizeof rbuf);
    if (len > 0) {
        char ipstr[64] = {0};
        skt_addr_ntop(ipstr, sizeof ipstr, ip);
        printf("recv data from %s:%u\n", ipstr, port);
        printf(">>%s\n", rbuf);
    }
    else {
        printf("skt_recvfrom failed.\n");
    }

    skt_close(sfd);
    return 0;
}
