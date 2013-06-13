#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef unsigned char byte_t;
typedef intptr_t int_t;
typedef uintptr_t uint_t;

#define OFFSET_OF(s, m)         ((int_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
                ({\
                     const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                     (type *)((byte_t *)p_mptr - OFFSET_OF(type, member));\
                 })

#define FALSE               0
#define TRUE                (!FALSE)

#define SRV_PORT            9797
#define WORKER_PROCESSES    1
#define CONNS_PER_PS        1000

#define unblocking_fd(fd)   fcntl(fd, \
                                  F_SETFL, \
                                  fcntl(fd, F_GETFL) | O_NONBLOCK)

// ***** 单链表 {{
typedef intptr_t list_t;

static inline
void add_node(list_t **pp_list, list_t *p_node)
{
    list_t *p_tmp = NULL;

    p_tmp = *pp_list;
    *p_node = (list_t)p_tmp;
    *pp_list = p_node;

    return;
}

static inline
void rm_node(list_t **pp_list, list_t *p_node)
{
    list_t **pp_curr = NULL;

    pp_curr = pp_list;
    while (*pp_curr) {
        if (p_node == *pp_curr) {
            *pp_curr = (list_t *)*p_node;
            *p_node = 0;

            break;
        }

        pp_curr = (list_t **)*pp_curr;
    }

    return;
}
// }} 单链表 *****


typedef struct {
    int m_fd; // 发生事件的描述符
    int m_overdue; // 事件过期标识
    list_t m_next;
} event_t;

typedef struct {
    list_t *mp_uselist;
    list_t *mp_freelist;
} event_mng_t;

static int s_master = 1;


static int worker_main(int lsn_fd)
{
    int epfd = 0;
    int tmp_err = 0;
    struct epoll_event epev = {};
    event_t *p_evs = NULL;
    event_t *p_ev = NULL;
    event_mng_t evmnger = {};
    
    errno = 0;
    epfd = epoll_create(CONNS_PER_PS);
    tmp_err = errno;
    if (-1 == epfd) {
        fprintf(stderr, "[ERROR] epoll_create failed: %d\n", tmp_err);

        return EXIT_FAILURE;
    }

    // 事件管理器初始化
    p_evs = (event_t *)malloc(sizeof(event_t) * CONNS_PER_PS);
    if (NULL == p_evs) {
        fprintf(stderr, "[ERROR] out of memory\n");

        return EXIT_FAILURE;
    }

    evmnger.mp_uselist = NULL;
    for (int i = 0; i < CONNS_PER_PS; ++i) {
        add_node(&evmnger.mp_freelist, &p_evs[i].m_next);
    }
    
    do {
        // 为监听套接字分配事件
        p_ev = CONTAINER_OF(&evmnger.mp_freelist[0], event_t, m_next);
        rm_node(&evmnger.mp_freelist, &p_ev->m_next);
        add_node(&evmnger.mp_uselist, &p_ev->m_next);
        p_ev->m_fd = lsn_fd;
        p_ev->m_overdue = FALSE;

        epev.events = EPOLLET | EPOLLIN;
        epev.data.ptr = p_ev;
        errno = 0;
        if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, lsn_fd, &epev)) {
            tmp_err = errno;
            fprintf(stderr, "[ERROR] epoll_ctl failed: %d\n", tmp_err);

        }
    } while (0);

    free(p_evs);
    (void)close(epfd);


    return EXIT_SUCCESS;
}

static int master_main(void)
{
    fprintf(stderr, "master process %d\n", getpid());
    return EXIT_SUCCESS;
}

static int listen_main(int lsn_fd)
{
    int tmp_err = 0;
    pid_t cpid = 0;

    if (-1 == listen(lsn_fd, SOMAXCONN)) {
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
        return worker_main(lsn_fd);
    }
}

static int bind_main(int lsn_fd)
{
    int tmp_err = 0;
    struct sockaddr_in srv_addr = {};

    // 非阻塞
    if (-1 == unblocking_fd(lsn_fd)) {
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
    if (-1 == bind(lsn_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr))) {
        tmp_err = errno;
        fprintf(stderr, "[ERROR] bind() failed: %d\n", tmp_err);

        return EXIT_FAILURE;
    }

    return listen_main(lsn_fd);
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

