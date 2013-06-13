#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define SRV_PORT            9797
#define WORKER_PROCESSES    4

#define unblocking_fd(fd)   fcntl(fd, fcntl(fd, F_GETFL) | O_NONBLOCK)

static int s_master = 1;


static int worker_main(void)
{
    return EXIT_SUCCESS;
}

static int master_main(void)
{
    return EXIT_SUCCESS;
}

static int listen_main(int sock)
{
    int tmp_err = 0;
    pid_t cpid = 0;

    if (-1 == listen(sock, SOMAXCONN)) {
        tmp_err = errno;
        fprintf(stderr, "[ERROR] listen() failed: %d\n", tmp_err);

        return EXIT_FAILURE;
    }

    for (int i = 0; i < WORKER_PROCESSES; ++i) {
        cpid = fork();

        if (-1 == cpid) {
            return EXIT_FAILURE;
        } else if (0 == cpid) {
            s_master = 0;
            break;
        } else {
            continue;
        }
    }

    if (s_master) {
        return master_main();
    } else {
        return worker_main();
    }
}

static int bind_main(int sock)
{
    int tmp_err = 0;
    struct sockaddr_in srv_addr = {};

    // 非阻塞
    if (-1 == unblocking_fd(sock)) {
        tmp_err = errno;
        fprintf(stderr,
                "[ERROR] set unblocking server fd failed: %d\n",
                tmp_err);

        return EXIT_FAILURE;
    }

    // 绑定
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
    if (s_master) {
        (void)close(srv_sock);
    }

    return rslt;
}

