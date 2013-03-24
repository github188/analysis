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


// shfs
#define LSN_PORT        8000

#define REUSEADDR       TRUE
#define KEEPALIVE       TRUE

extern int errno;

// 字符串
typedef struct {
    char *mp_data;
    int_t m_len;
} str_t;

#define STR_CPY(str_dst, dst_offset, str_src)   {\
            (void)memcpy(&str_dst.mp_data[dst_offset], \
                         str_src.mp_data, \
                         str_src.m_len);\
            str_dst.m_len = str_src.m_len + dst_offset;\
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
int offset_to_str(off_t value, str_t target, int n)
{
    int real_len = 0;
    off_t value_tmp = value;

    ASSERT(NULL != target.mp_data);

    if (value < 0) {
        real_len = -1;

        goto FINAL;
    }

    target.m_len = 0;
    ASSERT(0 == real_len);
    while ((0 != value_tmp) && (target.m_len < n)) {
        off_t next_value = value_tmp / 10;

        target.mp_data[target.m_len++] = value_tmp - next_value * 10 + '0';
        value_tmp = next_value;
        ++real_len;
    }
    reverse_str(target);

FINAL:
    return real_len;
}

// 默认访问文件
#define INDEX_FILE_STRING       "index.html"
static str_t const INDEX_FILE = {
    INDEX_FILE_STRING,
    sizeof(INDEX_FILE_STRING) - 1,
};

// 单链表
typedef intptr_t list_t;

static inline
void add_node(list_t **pp_list, list_t *p_node)
{
    list_t *p_tmp = NULL;

    ASSERT(NULL != pp_list);
    ASSERT(NULL != p_node);

    p_tmp = *pp_list;
    *p_node = (list_t)p_tmp;
    *pp_list = p_node;

    return;
}

static inline
void rm_node(list_t **pp_list, list_t *p_node)
{
    list_t **pp_curr = NULL;

    ASSERT(NULL != pp_list);
    ASSERT(NULL != p_node);

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
    list_t list[4] = {0};

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
int is_buf_empty(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    return (NULL == THIS->mp_data) ? TRUE : FALSE;
}

static inline
int is_buf_full(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    return (THIS->m_content_len < THIS->m_size - 1) ? FALSE : TRUE;
}

static inline
int doublesize_buf(buf_t *const THIS)
{
    int rslt = 0;
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

// 文件
typedef struct {
    int m_fd;
    off_t m_file_size;
    off_t m_read_size; // 已读取的大小
} file_t;

// ***** 处理连接请求 *****
// 监视的事件类型
typedef enum {
    SELECT_MR = 1,
    SELECT_MW = 1 << 1,
} select_type_t;

// connection列表
typedef struct {
    int m_cmnct_fd;
    file_t m_file; // 客户访问的文件
    list_t m_node;
    int m_select_type; // 监视类型
    struct sockaddr_in m_clt_addr; // 客户端地址
    buf_t m_recv_buf; // 接收缓冲
    buf_t m_send_buf; // 发送缓冲
    int m_sent_len; // 已发送长度
} client_t;

static int handle_accept(int lsn_fd, client_t *p_clt)
{
    int rslt = 0;

    ASSERT(NULL != p_clt);

    while (TRUE) { // 处理accept返回前连接夭折的情况
        int accept_errno = 0;
        socklen_t addrlen = 0;

        rslt = accept(lsn_fd,
                      (struct sockaddr *)&p_clt->m_clt_addr,
                      &addrlen);
        accept_errno = errno;
        if (rslt > 0) {
            break;
        } else if (-1 == rslt) {
            if ((EAGAIN == accept_errno)
                    || (ENOSYS == accept_errno)
                    || (ECONNABORTED == accept_errno))
            {
                continue;
            }

            break;
        } else {
            ASSERT(0);
        }
    }

    return rslt;
}

// ***** 处理http请求 *****
// http请求方法
static str_t const GET_METHOD = {
    "GET",
    sizeof("GET") - 1,
};
static str_t const POST_METHOD = {
    "POST",
    sizeof("POST") - 1,
};

// http请求
typedef struct {
    str_t const *mpc_requ_method;
    str_t m_filepath;
    char m_path_buf[PATH_MAX]; // 请求的文件路径
} http_request_t;

// http响应状态
typedef enum {
    RS_HTTP_200 = 200,
    RS_HTTP_403 = 403,
    RS_HTTP_404 = 404,
} response_status_t;


// html head
#define HTML32DOCTYPE           "<!DOCTYPE html>\r\n"

// server line
#define SERVER_NAME             "Server: shfs/1.0\r\n"

// status line
#define HTTP200                 "HTTP/1.1 200 OK\r\n"
#define HTTP403                 "HTTP/1.1 403 Forbidden\r\n"
#define HTTP403CONTENT          \
            HTML32DOCTYPE \
            "<head>\r\n" \
            "    <title>\r\n" \
            "        exception\r\n" \
            "    </title>\r\n" \
            "</head>\r\n" \
            "<body>\r\n" \
            "    <p>\r\n" \
            "        error code 403.\r\n" \
            "    </p>\r\n" \
            "    <p>\r\n" \
            "        message: not authorization.\r\n" \
            "    </p>\r\n" \
            "</body>\r\n"
#define HTTP403CONTENT_LEN      (sizeof(HTTP403CONTENT) - 1)
#define HTTP404                 "HTTP/1.1 404 File Not Found\r\n"
#define HTTP404CONTENT          \
            HTML32DOCTYPE \
            "<head>\r\n" \
            "    <title>\r\n" \
            "        exception\r\n" \
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
#define HTTP404CONTENT_LEN      (sizeof(HTTP404CONTENT) - 1)

typedef struct {
    int m_lsn_fd; // 监听套接字
    str_t m_path_root; // 根路径
    client_t *mp_client_cache; // 客户端缓存
    list_t *mp_free_clients; // 客户端空闲链
    list_t *mp_inuse_clients; // 客户端链
} context_t;


static int handle_http_request(context_t *p_context,
                               client_t *p_clt,
                               http_request_t *p_request)
{
    int rslt = 0;
    str_t filename = {
        NULL, 0,
    };
    buf_t *p_recv_buf = NULL;

    ASSERT(NULL != p_clt);
    p_recv_buf = &p_clt->m_recv_buf;

    // 初始化文件名
    filename.mp_data = p_recv_buf->mp_data;
    for (int i = 0; 0x00 != p_recv_buf->mp_data[i]; ++i) {
        if ('/' == filename.mp_data[0]) {
            break;
        }
        ++filename.mp_data;
    }
    if (filename.mp_data == p_recv_buf->mp_data) { // 没找到路径
        return -1;
    }
    ASSERT(0 == filename.m_len);
    for (int i = 0; 0x00 != filename.mp_data[i]; ++i) {
        if (0x20 == filename.mp_data[i]) {
            break;
        }
        ++filename.m_len;
    }
    if (filename.m_len > 128) { // 过长的文件名
        return -1;
    }

    // 初始化请求路径
    p_request->m_filepath.mp_data = p_request->m_path_buf; // 防止未初始化
    p_request->m_filepath.m_len = 0; // 防止未初始化
    p_request->mpc_requ_method = &GET_METHOD;
    STR_CPY(p_request->m_filepath, 0, p_context->m_path_root);
    if ('/' == p_request->m_filepath.mp_data[p_request->m_filepath.m_len - 1])
    {
        p_request->m_filepath.mp_data[p_request->m_filepath.m_len - 1] = '\0';
        --p_request->m_filepath.m_len;
    }
    STR_CPY(p_request->m_filepath, p_request->m_filepath.m_len, filename);

    clean_buf(&p_clt->m_recv_buf); // 清空接收缓冲

    return rslt;
}

static void http_response_404(client_t *p_clt)
{
    int ntow = 0;
    str_t len = {NULL};
    char content_len[32] = {0x00};
    buf_t *p_send_buf = NULL;

    ASSERT(NULL != p_clt);
    p_send_buf = &p_clt->m_send_buf;

    len.mp_data = content_len;
    len.m_len = 0;
    ASSERT(0 < offset_to_str(HTTP404CONTENT_LEN,
                             len,
                             ARRAY_COUNT(content_len)));
    ntow = snprintf(p_send_buf->mp_data,
                    p_send_buf->m_size,
                    HTTP404
                    SERVER_NAME
                    "Content-Length: %s\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Connection: keep-alive\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    HTTP404CONTENT,
                    len.mp_data);
    p_send_buf->m_content_len = MIN(ntow, p_send_buf->m_size);

    ASSERT(0 == p_clt->m_sent_len);
    if (p_send_buf->m_content_len > 0) {
        p_clt->m_select_type |= SELECT_MW; // 写事件
    }
}

static int handle_http_response(client_t *p_clt,
                                http_request_t *p_requ)
{
    int rslt = 0;
    int rdfd = 0;
    int ntow = 0;
    struct stat fp_stat = {0};
    buf_t *p_send_buf = NULL;
    char real_path[PATH_MAX] = {0x00};
    int real_path_len = 0;
    char *p_mimetype = NULL;
    char *p_filetype = NULL;
    str_t file_size = {NULL};
    char content_len[32] = {0x00};

    ASSERT(NULL != p_clt);
    p_send_buf = &p_clt->m_send_buf;

    if (-1 == stat(p_requ->m_filepath.mp_data, &fp_stat)) {
        // 404
        http_response_404(p_clt);

        return 0;
    }

    if (S_ISDIR(fp_stat.st_mode)) {
        if ('/' != p_requ->m_filepath.mp_data[p_requ->m_filepath.m_len - 1]) {
            p_requ->m_filepath.mp_data[p_requ->m_filepath.m_len] = '/';
            ++p_requ->m_filepath.m_len;
        }
        STR_CPY(p_requ->m_filepath, p_requ->m_filepath.m_len, INDEX_FILE);
    }

    if (NULL == realpath(p_requ->m_filepath.mp_data, real_path)) {
        // 404
        http_response_404(p_clt);

        return 0;
    }
    real_path_len = strlen(real_path);
    fprintf(stderr, "filename: %s\n", real_path);

    if (-1 == stat(p_requ->m_filepath.mp_data, &fp_stat)) {
        // 404
        http_response_404(p_clt);

        return 0;
    }

    rdfd = open(real_path, O_RDONLY);
    if (-1 == rdfd) {
        // 404
        http_response_404(p_clt);

        return 0;
    }
    p_filetype = &real_path[real_path_len - 1];
    while (real_path != p_filetype) {
        if ('.' == p_filetype[0]) {
            ++p_filetype;

            break;
        }
        --p_filetype;
    }

    if (real_path == p_filetype) {
        p_filetype = "unkown";
    }
    fprintf(stderr, "filetype: %s\n", p_filetype);

    if (0 == strcmp(p_filetype, "html")) {
        p_mimetype = "text/html";
    } else if (0 == strcmp(p_filetype, "png")) {
        p_mimetype = "image/png";
    } else if (0 == strcmp(p_filetype, "jpg")) {
        p_mimetype = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "jpeg")) {
        p_mimetype = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "txt")) {
        p_mimetype = "text/plain";
    } else if (0 == strcmp(p_filetype, "flv")) {
        p_mimetype = "video/flv";
    } else if (0 == strcmp(p_filetype, "mp4")) {
        p_mimetype = "video/mp4";
    } else if (0 == strcmp(p_filetype, "avi")) {
        p_mimetype = "video/avi";
    } else if (0 == strcmp(p_filetype, "rmvb")) {
        p_mimetype = "video/rmvb";
    } else {
        p_mimetype = "application/octet-stream";
    }

    file_size.mp_data = content_len;
    file_size.m_len = 0;
    ASSERT(0 < offset_to_str(fp_stat.st_size,
                             file_size,
                             ARRAY_COUNT(content_len)));
    fprintf(stderr, "file_size: %s\n", file_size.mp_data);

    ntow = snprintf(p_send_buf->mp_data,
                    p_send_buf->m_size,
                    HTTP200
                    SERVER_NAME
                    "Content-Length: 13\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Connection: keep-alive\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "Hello, World!");
    p_send_buf->m_content_len = MIN(ntow, p_send_buf->m_size);

    ASSERT(0 == p_clt->m_sent_len);
    if (p_send_buf->m_content_len > 0) {
        p_clt->m_select_type |= SELECT_MW; // 写事件
    }

    ASSERT(rdfd > 0);
    close(rdfd);

    return rslt;
}

static void ts_perror(char const *pc_msg, int error_no)
{
    char *p_desc = NULL;
    char err_buf[256] = {0x00};

    p_desc = strerror_r(error_no, err_buf, ARRAY_COUNT(err_buf));
    err_buf[ARRAY_COUNT(err_buf) - 1] = 0x00;
    fprintf(stderr, "[ERROR] [%d] %s: %s\n", error_no, pc_msg, p_desc);

    return;
}

static void handle_connection(context_t *p_context)
{
    int cmnct_fd = 0;
    client_t *p_clt = NULL;

    if (NULL == p_context->mp_free_clients) { // 无法再接受新连接
        return;
    }
    
    // 处理接受连接
    p_clt = CONTAINER_OF(p_context->mp_free_clients, client_t, m_node);
    cmnct_fd = handle_accept(p_context->m_lsn_fd, p_clt);
    if (-1 == cmnct_fd) {
        return;
    }

    // 非阻塞io
    if (-1 == fcntl(cmnct_fd,
                    F_SETFL,
                    fcntl(cmnct_fd, F_GETFL) | O_NONBLOCK))
    {
        ASSERT(0 == close(cmnct_fd));

        return;
    }


    // 初始化客户端
    p_clt->m_cmnct_fd = cmnct_fd;
    p_clt->m_select_type |= SELECT_MR; // 监视读事件
    if (is_buf_empty(&p_clt->m_recv_buf)) {
        if (-1 == create_buf(&p_clt->m_recv_buf, MIN_BUF_SIZE)) {
            ASSERT(0 == close(cmnct_fd));

            return;
        }
    } else {
        clean_buf(&p_clt->m_recv_buf);
    }
    if (is_buf_empty(&p_clt->m_send_buf)) {
        if (-1 == create_buf(&p_clt->m_send_buf, MIN_BUF_SIZE)) {
            ASSERT(0 == close(cmnct_fd));

            return;
        }
    } else {
        clean_buf(&p_clt->m_send_buf);
    }

    // 分配空闲结点并加入到客户端链表
    rm_node(&p_context->mp_free_clients,
            p_context->mp_free_clients);
    add_node(&p_context->mp_inuse_clients,
             &p_clt->m_node);

    return;
}

static void handle_disconnection(context_t *p_context,
                                 client_t *p_clt,
                                 int close_wait)
{
    ASSERT(NULL != p_context);

    // 断开连接
    if (close_wait) { // 被动断开
        ASSERT(0 == shutdown(p_clt->m_cmnct_fd, SHUT_RDWR));
    } else { // 主动断开
        ASSERT(0 == shutdown(p_clt->m_cmnct_fd, SHUT_WR));
        while (TRUE) {
            char buf_tmp[256] = {0x00};
                
            if (recv(p_clt->m_cmnct_fd,
                     buf_tmp,
                     ARRAY_COUNT(buf_tmp),
                     0) > 0)
            {
                continue;
            } else {
                ASSERT(0 == shutdown(p_clt->m_cmnct_fd, SHUT_RD));

                break;
            }
        }
    }

    // 释放资源
    ASSERT(0 == close(p_clt->m_cmnct_fd));
    rm_node(&p_context->mp_inuse_clients, &p_clt->m_node);
    add_node(&p_context->mp_free_clients, &p_clt->m_node);
    p_clt->m_select_type = 0;
    (void)memset(&p_clt->m_clt_addr, 0, sizeof(struct sockaddr_in));
    destroy_buf(&p_clt->m_recv_buf);
    destroy_buf(&p_clt->m_send_buf);
    p_clt->m_sent_len = 0;

    return;
}

static int handle_data_input(context_t *p_context, client_t *p_clt)
{
    int rslt = 0;

    ASSERT(NULL != p_clt);

    while (TRUE) {
        int recved_size = 0;
        buf_t *p_recv_buf = &p_clt->m_recv_buf;
        int recv_errno = 0;

        if (is_buf_full(&p_clt->m_recv_buf)) { // 缓冲满
            if (p_clt->m_recv_buf.m_size > 4096 * 255) {
                // 恶意请求
                rslt = -1;

                break;
            }
            if (-1 == doublesize_buf(&p_clt->m_recv_buf)) {
                rslt = -1;

                break;
            }
        }

        recved_size = recv(p_clt->m_cmnct_fd,
                           &p_recv_buf->mp_data[p_recv_buf->m_content_len],
                           p_recv_buf->m_size - p_recv_buf->m_content_len - 1,
                           0);
        recv_errno = errno;
        if (recved_size > 0) {
            p_recv_buf->m_content_len += recved_size;

            continue;
        } else if ((-1 == recved_size) && (EAGAIN == recv_errno)) {
            http_request_t hr = {NULL};

            ASSERT(0x00 == p_recv_buf->mp_data[p_recv_buf->m_content_len]);

            if (-1 == handle_http_request(p_context, p_clt, &hr)) {
                handle_disconnection(p_context, p_clt, FALSE); // 主动断开

                break;
            }
            handle_http_response(p_clt, &hr);

            break;
        } else {
            handle_disconnection(p_context, p_clt, TRUE); // 被动断开

            break;
        }
    }

    return rslt;
}

static void handle_read_events(context_t *p_context,
                               fd_set const *pc_fds_r,
                               int fd_max)
{
    for (int fd = 0; fd < fd_max + 1; ++fd) {
        if (!FD_ISSET(fd, pc_fds_r)) {
            continue;
        }

        if (fd == p_context->m_lsn_fd) { // 新连接
            handle_connection(p_context);
        } else {
            client_t *p_clt = NULL;

            // 查询client
            for (list_t *p_iter = p_context->mp_inuse_clients;
                 NULL != p_iter;
                 p_iter = (list_t *)*p_iter)
            {
                p_clt = CONTAINER_OF(p_iter, client_t, m_node);

                if (fd == p_clt->m_cmnct_fd) {
                    break;
                }
            }

            // 处理接收数据
            ASSERT(NULL != p_clt);
            if (-1 == handle_data_input(p_context, p_clt)) {
                // 主动断开连接
                handle_disconnection(p_context, p_clt, FALSE);
            }
        }
    }

    return;
}

static void handle_write_events(context_t *p_context,
                                fd_set const *pc_fds_w,
                                int fd_max)
{
    for (int fd = 0; fd < fd_max + 1; ++fd) {
        client_t *p_clt = NULL;
        int sent_size = 0;

        if (!FD_ISSET(fd, pc_fds_w)) {
            continue;
        }

        // 查询client
        for (list_t *p_iter = p_context->mp_inuse_clients;
             NULL != p_iter;
             p_iter = (list_t *)*p_iter)
        {
            p_clt = CONTAINER_OF(p_iter, client_t, m_node);

            if (fd == p_clt->m_cmnct_fd) {
                break;
            }
        }
        p_clt->m_select_type &= ~SELECT_MW; // 清除写事件标识

        ASSERT(0 == sent_size);
        while (TRUE) {
            int send_errno = 0;
            int left_size = p_clt->m_send_buf.m_content_len - sent_size;

            if (0 == left_size) { // 发送完毕
                p_clt->m_sent_len = 0;
                clean_buf(&p_clt->m_send_buf);

                break;
            }
            sent_size = send(p_clt->m_cmnct_fd,
                             &p_clt->m_send_buf.mp_data[sent_size],
                             MIN(4096, left_size),
                             0);
            send_errno = errno;
            if (sent_size > 0) {
                p_clt->m_sent_len += sent_size;

                continue;
            } else if ((-1 == sent_size) && (EAGAIN == send_errno)) {
                p_clt->m_select_type |= SELECT_MW; // 重设写事件标识，下次再发

                break;
            } else {
                handle_disconnection(p_context, p_clt, FALSE); // 主动断开

                break;
            }
        }
    }

    return;
}

static void handle_events(context_t *p_context,
                          fd_set const *pc_fds_r,
                          fd_set const *pc_fds_w,
                          int fd_max)
{
    handle_read_events(p_context, pc_fds_r, fd_max);
    handle_write_events(p_context, pc_fds_w, fd_max);

    return;
}

static int event_loop(context_t *p_context)
{
    int rslt = 0;
    int fd_max = 0;
    int nevents = 0;
    fd_set fds_r = {{0}};
    fd_set fds_w = {{0}};
    struct timeval io_wait_tv = {
        0, 0,
    };

    while (TRUE) {
        fd_max = p_context->m_lsn_fd;

        FD_ZERO(&fds_r);
        FD_ZERO(&fds_w);
        
        // 重新设置描述符集
        FD_SET(p_context->m_lsn_fd, &fds_r);
        for (list_t *p_iter = p_context->mp_inuse_clients;
             NULL != p_iter;
             p_iter = (list_t *)*p_iter)
        {
            struct stat tmp_stat = {0};
            client_t *p_clt = CONTAINER_OF(p_iter, client_t, m_node);

            if (0 == fstat(p_clt->m_cmnct_fd, &tmp_stat)) {
                if (p_clt->m_cmnct_fd > fd_max) {
                    fd_max = p_clt->m_cmnct_fd;
                }
                if (p_clt->m_select_type & SELECT_MR) {
                    FD_SET(p_clt->m_cmnct_fd, &fds_r);
                }
                if (p_clt->m_select_type & SELECT_MW) {
                    #define CONDITION       \
                        (p_clt->m_sent_len < p_clt->m_send_buf.m_content_len)
                    ASSERT(CONDITION);
                    #undef CONDITION

                    FD_SET(p_clt->m_cmnct_fd, &fds_w);
                }
            } else {
                ASSERT(0);
            }
        }

        // 重置定时器
        io_wait_tv.tv_sec = 7;
        io_wait_tv.tv_usec = 20 * 1000; // 20毫秒定时器
        
        // 监视事件
        nevents = select(fd_max + 1, &fds_r, &fds_w, NULL, &io_wait_tv);
        if (nevents > 0) {
            handle_events(p_context, &fds_r, &fds_w, fd_max);
        } else if (0 == nevents) { // 超时
            continue;
        } else {
            rslt = -1;

            break;
        }
    }

    return rslt;
}

static int init_lsn_fd(int lsn_fd)
{
    int reuseaddr = REUSEADDR;
    int keepalive = KEEPALIVE;
    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(LSN_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_zero = {0},
    };

    // 非阻塞io
    if (-1 == fcntl(lsn_fd,
                    F_SETFL,
                    fcntl(lsn_fd, F_GETFL) | O_NONBLOCK))
    {
        return -1;
    }

    // 地址重用
    if (-1 == setsockopt(lsn_fd,
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &reuseaddr,
                         sizeof(reuseaddr)))
    {
        ts_perror("reuse addr failed", errno);

        return -1;
    }

    // 连接保持
    if (-1 == setsockopt(lsn_fd,
                         SOL_SOCKET,
                         SO_KEEPALIVE,
                         &keepalive,
                         sizeof(keepalive)))
    {
        return -1;
    }

    // 绑定ip端口
    if (-1 == bind(lsn_fd,
                   (struct sockaddr *)&srv_addr,
                   sizeof(srv_addr)))
    {
        return -1;
    }

    // 监听
    if (-1 == listen(lsn_fd, SOMAXCONN)) {
        return -1;
    }

    return 0;
}


static void destroy_context(context_t *p_context);
static int build_context(context_t *p_context, str_t const PATH_ROOT);

int build_context(context_t *p_context, str_t const PATH_ROOT)
{
    int lsn_fd = 0;
    struct rlimit rlmt = {0};
    client_t *p_client_cache = NULL;
    list_t *p_free_clients = NULL;

    // 创建监听套接字
    lsn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == lsn_fd) {
        return -1;
    }

    // 初始化监听套接字
    if (-1 == init_lsn_fd(lsn_fd)) {
        ASSERT(0 == close(lsn_fd));

        return -1;
    }

    // 获得能打开的最大描述符数目
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlmt)) {
        ASSERT(0 == close(lsn_fd));

        return -1;
    }

    // 申请客户端结构缓存
    p_client_cache = calloc(rlmt.rlim_cur, sizeof(client_t));

    if (NULL == p_client_cache) {
        ASSERT(0 == close(lsn_fd));

        return -1;
    }

    // 建立客户端空闲链
    for (int i = 0; i < rlmt.rlim_cur; ++i) {
        add_node(&p_free_clients,
                 &p_client_cache[i].m_node);
    }

    // 填充上下文
    p_context->m_lsn_fd = lsn_fd;
    p_context->m_path_root = PATH_ROOT;
    p_context->mp_client_cache = p_client_cache; // 客户端缓存
    p_context->mp_free_clients = p_free_clients; // 客户端空闲链
    p_context->mp_inuse_clients = NULL; // 客户端链

    return 0;
}

void destroy_context(context_t *p_context)
{
    // 关闭监听
    ASSERT(0 == close(p_context->m_lsn_fd));

    // 释放客户端结构缓存
    free(p_context->mp_client_cache);

    return;
}

static int shfs_main(str_t const PATH_ROOT)
{
    context_t context = {0};

    // 创建运行上下文
    if (-1 == build_context(&context, PATH_ROOT)) {
        return -1;
    }

    // 事件循环
    if (-1 == event_loop(&context)) {
        destroy_context(&context);

        return -1;
    }

    // 销毁运行上下文
    destroy_context(&context);

    return 0;
}

int main(int argc, char *argv[])
{
    str_t path_root = {NULL};

    if (1 == argc) {
        fprintf(stderr, "usage: shfs filename\n");

        return 0;
    }
    path_root.mp_data = argv[1];
    path_root.m_len = strlen(argv[1]);

    return shfs_main(path_root);
}
