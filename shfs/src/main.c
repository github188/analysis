#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <netinet/in.h>


#ifndef __GNUC__
    #define __asm__     asm
#endif
#ifndef __GNUC__
    #define __inline__  inline
#endif
#ifndef __GNUC__
    #define __typeof__  typeof
#endif


// common types
typedef signed char byte_t;
typedef unsigned char ubyte_t;
typedef intptr_t int_t;
typedef uintptr_t uint_t;

typedef intptr_t bool_t;
typedef intptr_t handle_t;


#define FALSE           (0)
#define TRUE            (1)

#define EXEC_SUCCEED    (0)
#define EXEC_FAILURE    (-1)


// common macros
#define NONE            ((void)0)

#ifdef NDEBUG
    #ifdef NASSERT
        #define ASSERT(cond)    do {} while (0)
    #else
        #define ASSERT(cond)        \
                    do {\
                        if (!(cond)) {\
                            (void)fprintf(stderr, \
                                          "[Assert Failed] %s: %d\n", \
                                          __FILE__, \
                                          __LINE__);\
                            abort();\
                        }\
                    } while (0)
    #endif // NASSERT
#else
    #define ASSERT(cond)        assert(cond)
#endif // NDEBUG

#define OFFSET_OF(s, m)         ((int_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
            ({\
                const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                (type *)((byte_t *)p_mptr - OFFSET_OF(type, member));\
            })
#define ARRAY_COUNT(a)          (sizeof(a) / sizeof((a)[0]))

#define HOWMANY(x, y)           (((x) + ((y) - 1)) / (y))

#define ABS(x)                  \
            (\
                ((x) ^ ((x) >> (sizeof(x) * 8 - 1))) \
                    - ((x) >> (sizeof(x) * 8 - 1))\
            )

#define SWAP(a, b)              \
            do {\
                __typeof__(a) tmp = (b);\
                (b) = (a);\
                (a) = tmp;\
            } while (0)

#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define MID(a, b, c)            \
            (\
                ((((a) - (b)) * ((b) - (c))) > 0) \
                    ? (b) \
                    : (((((b) - (a)) * ((a) - (c))) > 0) ? a : c)\
            )
#define MIN(a, b)               (((a) > (b)) ? (b) : (a))

#define DO_NOTHING()            do {} while (0)

// 比较交换接口
enum {
    CMP_GREATER_THAN = 2,
    CMP_EQUAL = 3,
    CMP_LESS_THAN = 5,
};

typedef struct {
    int_t (*mpf_compare)(void const *, void const *);
    void (*mpf_swap)(void *, void *);
} compare_swap_t;


#define LSN_PORT        8000

#define REUSEADDR       TRUE


int main(int argc, char *argv[])
{
    int rslt = 0;
    int_t lsn_fd = 0;
    int_t reuseaddr = REUSEADDR;
    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(LSN_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_zero = {0},
    };
    struct rlimit rlmt = {0};
    struct timeval io_wait_tv = {
        5, 0,
    };
    int_t sockets_max = 0;
    fd_set fds = {{0}};
    int_t select_err = FALSE;

    FD_ZERO(&fds);
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlmt)) {
        goto GETRLIMIT_ERR;
    }
    lsn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == lsn_fd) {
        goto SOCKET_ERR;
    }
    if (-1 == setsockopt(lsn_fd,
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &reuseaddr,
                         sizeof(reuseaddr)))
    {
        goto SETSOCKOPT_ERR;
    }
    if (-1 == bind(lsn_fd,
                   (struct sockaddr *)&srv_addr,
                   sizeof(srv_addr)))
    {
        goto BIND_ERR;
    }
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlmt)) {
        goto GETRLIMIT_ERR;
    }
    sockets_max = (int_t)rlmt.rlim_cur;
    fprintf(stdout, "max sockets: %d\n", sockets_max);
    if (-1 == listen(lsn_fd, SOMAXCONN)) {
        goto LISTEN_ERR;
    }
    fprintf(stdout, "max backlog: %d\n", SOMAXCONN);

    while (TRUE) {
        int nevents = 0;
        
        FD_SET(lsn_fd, &fds);
        nevents = select(FD_SETSIZE + 1, &fds, NULL, NULL, &io_wait_tv);

        if (0 == nevents) { // time out
            continue;
        }
        if (-1 == nevents) {
            select_err = TRUE;

            break;
        }

        for (int i = 0; i < fds.fd_count; ++i) {
            if (!FD_ISSET(fds.fd_array[i], &fds)) {
                continue;
            }

            if (fd.fd_array[i] == lsn_fd) {
            } else {
            }
        }
    }
    
    if (select_err) {
        goto SELECT_ERR;
    }

    do {
        break;

SELECT_ERR:

LISTEN_ERR:

GETRLIMIT_ERR:

BIND_ERR:

SETSOCKOPT_ERR:
        close(lsn_fd);

SOCKET_ERR:
        rslt = -1;
    } while (0);

    return rslt;
}
