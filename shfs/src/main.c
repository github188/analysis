#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <linux/limits.h>


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

extern int errno;

// 字符串
typedef struct {
    char *mp_data;
    int_t m_len;
} str_t;

#define STR_CPY(str_dst, dst_offset, str_src)   {\
            (void)memcpy(str_dst.mp_data + dst_offset, \
                         str_src.mp_data, \
                         str_src.m_len);\
            str_dst.m_len = str_src.m_len;\
            str_dst.mp_data[str_dst.m_len] = '\0';\
        }

static inline
void reverse_str(str_t target)
{
    int i = 0;
    int j = 0;

    ASSERT(NULL != target.mp_data);
    ASSERT(target.m_len > 0);

    for (i = 0, j = target.m_len - 1; i < j; ++i, --j) {
        char tmp = target.mp_data[i];

        target.mp_data[i] = target.mp_data[j];
        target.mp_data[j] = tmp;
    }
}

static inline
int offset_to_str(off_t value, str_t target)
{
    int iter = 0;
    int real_len = 0;
    off_t value_tmp = value;

    ASSERT(value > 0);
    ASSERT(NULL != target.mp_data);
    ASSERT(target.m_len > 0);

    ASSERT(0 == iter);
    ASSERT(0 == real_len);
    while ((0 != value_tmp) && (iter < target.m_len)) {
        off_t next_value = value_tmp / 10;

        target.mp_data[iter++] = value_tmp - next_value * 10 + '0';
        value_tmp = next_value;
        ++real_len;
    }

    return real_len;
}

// 单链表
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

#ifndef NDEBUG
void test(void)
{
    list_t *p_iter = NULL;
    static list_t *p_list_head = NULL;
    list_t list[4];

    /*p_list_head = &list[0];
    list[0] = (list_t)&list[1];
    list[1] = (list_t)&list[2];
    list[2] = (list_t)&list[3];
    list[3] = (list_t)NULL;*/
    for (int i = 0; i < 4; ++i) {
        add_node(&p_list_head, &list[i]);
    }

    fprintf(stderr, "addr of p_list_head: %p\n", &p_list_head);
    p_iter = p_list_head;
    while (NULL != p_iter) {
        fprintf(stderr, "%p[%x]\n", p_iter, *p_iter);
        p_iter = (list_t *)*p_iter;
    }

    fprintf(stderr, "############\n");

    rm_node(&p_list_head, &list[1]);
    p_iter = p_list_head;
    while (NULL != p_iter) {
        fprintf(stderr, "%p[%x]\n", p_iter, *p_iter);
        p_iter = (list_t *)*p_iter;
    }
}
#endif // NDEBUG


// 缓冲区
#define MIN_BUF_SIZE            4096

typedef struct {
    char *mp_data;
    ssize_t m_size;
    ssize_t m_content_len;
} buf_t;

static inline
int_t create_buf(buf_t *const THIS, ssize_t size)
{
    int_t rslt = 0;

    ASSERT(NULL != THIS);

    if (size < MIN_BUF_SIZE) {
        size = MIN_BUF_SIZE;
    }

    THIS->mp_data = (char *)malloc(size);
    if (NULL == THIS->mp_data) {
        goto FAILED;
    }
    THIS->m_size = size;
    THIS->m_content_len = 0;

    do {
        break;

FAILED:
        rslt = -1;
    } while (0);

    return rslt;
}

static inline
void clean_buf(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    (void)memset(THIS->mp_data, 0, THIS->m_size);
    THIS->m_content_len = 0;

    return;
}

static inline
int_t is_buf_empty(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    return (NULL == THIS->mp_data) ? TRUE : FALSE;
}

static inline
int_t doublesize_buf(buf_t *const THIS)
{
    int_t rslt = 0;
    char *p_tmp = NULL;
    ssize_t tmp_size = 2 * THIS->m_size;

    p_tmp = (char *)malloc(tmp_size);
    if (NULL == p_tmp) {
        goto FAILED;
    }

    (void)memset(&p_tmp[0], 0, tmp_size);
    if (NULL != THIS->mp_data) {
        (void)memcpy(&p_tmp[0], &THIS->mp_data[0], THIS->m_content_len);

        free(THIS->mp_data);
    }
    THIS->mp_data = p_tmp;
    THIS->m_size = tmp_size;

    do {
        break;

FAILED:
        rslt = -1;
    } while (0);

    return rslt;
}

static inline
void destroy_buf(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    free(THIS->mp_data);
    THIS->mp_data = NULL;
    THIS->m_size = 0;
    THIS->m_content_len = 0;
}

// http请求
typedef struct {
    str_t m_location;
} http_request_t;

// connection列表
typedef struct {
    int m_cmnct_fd;
    list_t m_node;
    buf_t m_recv_buf; // 接收缓冲
    buf_t m_send_buf; // 发送缓冲
    int m_sent_len; // 已发送长度
} client_t;

#define HTML32DOCTYPE           \
            "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\r\n"

// status line
#define HTTP200                 "HTTP/1.1 200 OK\r\n"
#define HTTP404                 "HTTP/1.1 404 File Not Found\r\n"
#define HTTP404CONTENT          \
            HTML32DOCTYPE \
            "<head>\r\n" \
            "    <title>\r\n" \
            "        error response\r\n" \
            "    </title>\r\n" \
            "</head>\r\n" \
            "<body>\r\n" \
            "    <p>\r\n" \
            "        error code 404.\r\n" \
            "    </p>\r\n" \
            "    <p>\r\n" \
            "        message: file not found.\r\n" \
            "    </p>\r\n" \
            "</body>\r\n"

// server line
#define SERVER_NAME             "Server: shfs/1.0\r\n"

#define INDEX_FILE_STRING       "index.html"
static str_t const INDEX_FILE = {
    INDEX_FILE_STRING,
    sizeof(INDEX_FILE_STRING) - 1,
};
static str_t path_root = {
    NULL, 0,
};

int main(int argc, char *argv[])
{
    int rslt = 0;
    int lsn_fd = 0;
    int reuseaddr = REUSEADDR;
    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(LSN_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_zero = {0},
    };
    struct rlimit rlmt = {0};
    struct timeval io_wait_tv = {
        0, 0,
    };
    int sockets_max = 0;
    fd_set fds = {{0}};

    client_t *p_client_cache = NULL;
    list_t *p_free_clients = NULL; // 空闲客户端
    list_t *p_clients = NULL; // 客户端列表

    int loop_err = 0;

#ifndef NDEBUG
    test();
#endif // NDEBUG

    if (1 == argc) {
        fprintf(stderr, "usage: shfs filename\n");

        goto FINAL;
    } else {
        path_root.mp_data = argv[1];
        path_root.m_len = strlen(argv[1]);
    }

    // 获得能打开的最大描述符数目
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlmt)) {
        goto GETRLIMIT_ERR;
    }
    sockets_max = rlmt.rlim_cur;
    fprintf(stdout, "[INFO] max sockets: %d\n", sockets_max);

    // 客户端结构缓存
    p_client_cache = calloc(sockets_max, sizeof(client_t));
    if (NULL == p_client_cache) {
        goto CALLOC_ERR;
    }

    // 建立客户端空闲链
    for (int i = 0; i < sockets_max; ++i) {
        add_node(&p_free_clients, &p_client_cache[i].m_node);
    }

    // 创建监听套接字
    lsn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == lsn_fd) {
        goto SOCKET_ERR;
    }
    fprintf(stdout, "[INFO] listen fd: %d\n", lsn_fd);

    // 地址重用
    if (-1 == setsockopt(lsn_fd,
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &reuseaddr,
                         sizeof(reuseaddr)))
    {
        goto SETSOCKOPT_ERR;
    }

    // 绑定ip端口
    if (-1 == bind(lsn_fd,
                   (struct sockaddr *)&srv_addr,
                   sizeof(srv_addr)))
    {
        fprintf(stderr, "[ERROR] bind server port %d failed!\n", LSN_PORT);

        goto BIND_ERR;
    }

    // 监听
    if (-1 == listen(lsn_fd, SOMAXCONN)) {
        goto LISTEN_ERR;
    }
    fprintf(stdout,
            "[INFO] listening on %d, max backlog: %d\n",
            LSN_PORT,
            SOMAXCONN);

    // 事件循环
    while (TRUE) {
        int nevents = 0;

        // 重置描述符集
        FD_ZERO(&fds);
        FD_SET(lsn_fd, &fds);
        for (list_t *p_iter = p_clients;
             NULL != p_iter;
             p_iter = (list_t *)*p_iter)
        {
            struct stat tmp_stat = {0};
            client_t *p_clt = CONTAINER_OF(p_iter, client_t, m_node);

            if (0 == fstat(p_clt->m_cmnct_fd, &tmp_stat)) {
                FD_SET(p_clt->m_cmnct_fd, &fds);
            } else {
                fprintf(stderr, "[BUG] bad fd: %d\n", p_clt->m_cmnct_fd);
            }
        }

        // 重置定时器
        io_wait_tv.tv_sec = 10;
        io_wait_tv.tv_usec = 0;

        nevents = select(sockets_max + 1, &fds, NULL, NULL, &io_wait_tv);

        if (0 == nevents) { // time out
            continue;
        }
        if (-1 == nevents) {
            loop_err = TRUE;
            fprintf(stderr, "[ERROR] select failed: %s.\n", strerror(errno));

            break;
        }

        for (int fd = 0; fd < sockets_max; ++fd) {
            if (!FD_ISSET(fd, &fds)) {
                continue;
            }

            if (fd == lsn_fd) { // connection input
                int cmnct_fd = 0;
                client_t *p_clt = NULL;

                if (NULL == p_free_clients) { // 无法再接受新连接
                    continue;
                }
                cmnct_fd = accept(lsn_fd, NULL, NULL);
                if (-1 == cmnct_fd) {
                    loop_err = TRUE;
                    fprintf(stderr, "[ERROR] accept failed.\n");

                    break;
                }

                // 非阻塞io
                if (-1 == fcntl(cmnct_fd,
                                F_SETFL,
                                fcntl(cmnct_fd, F_GETFL) | O_NONBLOCK))
                {
                    loop_err = TRUE;
                    fprintf(stderr, "[ERROR] fcntl failed.\n");

                    break;
                }

                // 分配空闲结点
                p_clt = CONTAINER_OF(p_free_clients, client_t, m_node);
                rm_node(&p_free_clients, p_free_clients);

                // 加入到客户端列表
                p_clt->m_cmnct_fd = cmnct_fd;

                // 第一次使用分配缓冲
                if (is_buf_empty(&p_clt->m_recv_buf)) {
                    if (-1 == create_buf(&p_clt->m_recv_buf, MIN_BUF_SIZE)) {
                        loop_err = TRUE;
                        fprintf(stderr, "[ERROR] create recv buf failed!\n");

                        break;
                    }
                }
                if (is_buf_empty(&p_clt->m_send_buf)) {
                    if (-1 == create_buf(&p_clt->m_send_buf, MIN_BUF_SIZE)) {
                        loop_err = TRUE;
                        fprintf(stderr, "[ERROR] create send buf failed!\n");

                        break;
                    }
                }
                add_node(&p_clients, &p_clt->m_node);
            } else { // data input

                int_t recved_size = 0;
                int_t recv_errno = 0;
                client_t *p_clt = NULL;
                buf_t *p_recv_buf = NULL;
                buf_t *p_send_buf = NULL;

                for (list_t *p_iter = p_clients;
                     NULL != p_iter;
                     p_iter = (list_t *)*p_iter)
                {
                    p_clt = CONTAINER_OF(p_iter, client_t, m_node);

                    if (fd == p_clt->m_cmnct_fd) {
                        p_recv_buf = &p_clt->m_recv_buf;
                        p_send_buf = &p_clt->m_send_buf;

                        break;
                    }
                }

                // ***** recv *****
                ASSERT(NULL != p_recv_buf);
                if (is_buf_empty(p_recv_buf)) {
                    if (-1 == doublesize_buf(p_recv_buf)) {
                        loop_err = TRUE;

                        break;
                    }
                }

                ASSERT(!is_buf_empty(p_recv_buf));
                recved_size = recv(fd,
                                   &p_recv_buf->mp_data[
                                       p_recv_buf->m_content_len],
                                   p_recv_buf->m_size
                                       - p_recv_buf->m_content_len - 1,
                                   0);
                recv_errno = errno;
                if (recved_size > 0) { // 还有数据要收
                    p_recv_buf->m_content_len += recved_size;

                    continue; // 轮到下一描述符
                } else if ((-1 == recved_size) && (EAGAIN == recv_errno)) {
                    char fn_buf[PATH_MAX] = {0};
                    str_t filename = {
                        fn_buf, 0,
                    };
                    int rdfd = 0;
                    struct stat url_stat = {0};
                    http_request_t requ = {
                        {
                            NULL, 0,
                        },
                    };

                    // 收完数据
                    fprintf(stderr,
                            "[fd:%d] %s\n",
                            fd,
                            p_recv_buf->mp_data);

                    requ.m_location.mp_data = p_recv_buf->mp_data;
                    while ('/' != *requ.m_location.mp_data) {
                        ++requ.m_location.mp_data;
                    }
                    ASSERT(0 == requ.m_location.m_len);
                    for (int i = 0;
                         0x0a != requ.m_location.mp_data[i];
                         ++i)
                    {
                        ++requ.m_location.m_len;
                    }
                    STR_CPY(filename, 0, path_root);
                    if ('/' == filename.mp_data[filename.m_len - 1])
                    {
                        filename.mp_data[filename.m_len] = '\0';
                        --filename.m_len;
                    };
                    STR_CPY(filename, filename.m_len, requ.m_location);
                    if (-1 == stat(filename.mp_data, &url_stat)) {
                        // 404
                        clean_buf(&p_clt->m_recv_buf);
                        clean_buf(&p_clt->m_send_buf);
                        rm_node(&p_clients, &p_clt->m_node);
                        add_node(&p_free_clients, &p_clt->m_node);
                        close(fd);

                        continue;
                    }
                    if (S_ISDIR(url_stat.st_mode))
                    {
                        if ('/' != filename.mp_data[filename.m_len - 1]) {
                            filename.mp_data[filename.m_len] = '/';
                            ++filename.m_len;
                        }
                        STR_CPY(filename, filename.m_len, INDEX_FILE);
                    }
                    fprintf(stderr, "filename: %s\n", filename.mp_data);
                    rdfd = open(filename.mp_data, O_RDONLY);
                    if (-1 == rdfd) {
                        clean_buf(&p_clt->m_recv_buf);
                        clean_buf(&p_clt->m_send_buf);
                        rm_node(&p_clients, &p_clt->m_node);
                        add_node(&p_free_clients, &p_clt->m_node);
                        close(fd);

                        continue;
                    }

                    snprintf(p_send_buf->mp_data,
                                 p_send_buf->m_size,
                                 HTTP200
                                 SERVER_NAME
                                 "Content-Length: 13\r\n"
                                 "Cache-Control: no-cache\r\n"
                                 "Connection: keep-alive\r\n"
                                 "Content-Type: text/html\r\n\r\n");

                    close(rdfd);
                    clean_buf(p_recv_buf);
                } else {
                    // 该套接字上发生错误，清理资源关闭连接
                    clean_buf(&p_clt->m_recv_buf);
                    clean_buf(&p_clt->m_send_buf);
                    rm_node(&p_clients, &p_clt->m_node);
                    add_node(&p_free_clients, &p_clt->m_node);
                    close(fd);

                    continue; // 轮到下一描述符
                }

                // ***** send *****
#define BUFFER      HTTP200 \
                    SERVER_NAME \
                    "Content-Length: 13\r\n" \
                    "Cache-Control: no-cache\r\n" \
                    "Connection: keep-alive\r\n" \
                    "Content-Type: text/html\r\n\r\n" \
                    "Hello, World!"
                send(fd, BUFFER, sizeof(BUFFER), 0);
#undef BUFFER
            } // end of data input
        } // end of fd set ergodic

        if (loop_err) {
            break;
        }
    }

    if (loop_err) {
        goto LOOP_ERR;
    }

    do {
        break;

LOOP_ERR:
        for (list_t *p_iter = p_clients;
             NULL != p_iter;
             p_iter = (list_t *)*p_iter)
        {
            client_t *p_clt = CONTAINER_OF(p_iter, client_t, m_node);

            destroy_buf(&p_clt->m_recv_buf);
            destroy_buf(&p_clt->m_send_buf);
            close(p_clt->m_cmnct_fd);
        }
        FD_ZERO(&fds);

LISTEN_ERR:

BIND_ERR:

SETSOCKOPT_ERR:
        close(lsn_fd);

SOCKET_ERR:
        free(p_client_cache);

CALLOC_ERR:

GETRLIMIT_ERR:
        rslt = -1;
    } while (0);

FINAL:
    fprintf(stderr, "exit: %d\n", rslt);

    return rslt;
}
