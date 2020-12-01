#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>


#include "skt.h"


int main(int argc, char *argv[])
{
    int sfd;
    size_t len;

    sfd = skt_tcp_connect("127.0.0.1", 3600);
    if (sfd < 0) {
        printf("error\n");
        return -1;
    }
    printf(">>>%d\n", sfd);

    len = skt_rio_writen(sfd, "123456\n", 7);
    if (len != 6) {
        skt_close(sfd);
        return -1;
    }
    skt_close(sfd);

#if 0
    sfd = skt_udp_connect("0.0.0.0", 8888);
    len = skt_send(sfd, "123456\n", 7);
    if (len < 0)
        perror("skt_send failed");

    skt_close(sfd);

    return 0;
#endif
}
