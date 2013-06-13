#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define SRV_PORT        9797


static int listen_main(sock)
{
    return EXIT_SUCCESS;
}

static int bind_main(int sock)
{
    int tmp_err = 0;
    struct sockaddr_in srv_addr = {};

    (void)memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(SRV_PORT);
    errno = 0;
    if (-1 == bind(sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr))) {
        tmp_err = errno;
        fprintf(stderr, "[ERROR] bind() failed: %d\n", tmp_err);

        return EXIT_FAILURE;
    }

    return listen_main(sock);
}

int main(int argc, char *argv[])
{
    int rslt = 0;
    int srv_sock = 0;
    int tmp_err = 0;

    errno = 0;
    srv_sock = socket(PF_INET, SOCK_STREAM, 0);
    tmp_err = errno;
    if (-1 == srv_sock) {
        fprintf(stderr, "[ERROR] socket() failed: %d\n", tmp_err);

        return EXIT_FAILURE;
    }
    rslt = bind_main(srv_sock);
    (void)close(srv_sock);

    return rslt;
}

