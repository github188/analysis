#include "shfs.h"


#define LSN_PORT        8000

#define REUSEADDR       TRUE
#define KEEPALIVE       TRUE

#define SPACE           0x20

extern int errno;


// 字符串哈希
static int str_hash(str_t str)
{
    int key = 0;

    ASSERT(NULL != str.mp_data);
    ASSERT(str.m_len > 0);

    for (int i = 0; 0x00 != str.mp_data[i]; ++i) {
        if (i < str.m_len) {
            key = (key << 5) + str.mp_data[i];
        } else {
            break;
        }
    }

    return key;
}

// 路径
typedef struct {
    str_t m_path;
    char m_buf[PATH_MAX];
} path_t;

static inline void init_path(path_t *const THIS)
{
    ASSERT(NULL != THIS);

    THIS->m_path.mp_data = THIS->m_buf;
    THIS->m_path.m_len = 0;
}


// 默认访问文件
#define INDEX_FILE_STRING       "index.html"
static str_t const INDEX_FILE = {
    INDEX_FILE_STRING,
    sizeof(INDEX_FILE_STRING) - 1,
};

// http请求方法
typedef enum {
    RM_HTTP_GET = 0x124F4,   // 由str_hash算出的哈希值，"GET"，以下类推
    RM_HTTP_POST = 0x2946B4,
    RM_HTTP_HEAD = 0x251C64,
} requ_method_t;

// http响应状态
typedef enum {
    RS_HTTP_200 = 200,
    RS_HTTP_403 = 403,
    RS_HTTP_404 = 404,
} resp_status_t;

// 监视的事件类型
typedef enum {
    SELECT_MR = 1,
    SELECT_MW = 1 << 1,
} select_type_t;


// 文件
typedef struct {
    int m_fd;
    off_t m_size;
} file_t;

// http请求
typedef struct {
    int m_requ_method;
} http_requ_t;

// http响应
typedef struct {
    int m_resp_status; // 响应状态
    file_t m_file;
    char const *mpc_mime; // mime类型
    off_t m_sent_size; // 已发送大小
    off_t m_total_size; // 总大小
} http_resp_t;


// connection列表
typedef struct {
    int m_cmnct_fd;
    int m_select_type; // 监视类型
    http_requ_t m_requ; // http请求
    http_resp_t m_resp; // http响应
    struct sockaddr_in m_clt_addr; // 客户端地址
    buf_t m_recv_buf; // 接收缓冲
    buf_t m_send_buf; // 发送缓冲
    list_t m_node;
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

static int send_buf(int fd, buf_t *p_buf)
{
    int sent = 0;
    int ntow = 0;

    ASSERT(NULL != p_buf);

    sent = 0;
    while (TRUE) {
        int sent_size = 0;
        int sent_errno = 0;

        ntow = p_buf->m_size - p_buf->m_seek;
        if (0 == ntow) {
            break;
        }

        sent_size = send(fd,
                         &p_buf->mp_data[p_buf->m_seek],
                         ntow,
                         0);
        sent_errno = errno;

        if (sent_size > 0) {
            p_buf->m_seek += sent_size;
            sent += sent_size;

            continue;
        } else if ((-1 == sent_size) &&(EAGAIN == sent_errno)) {
            break;
        } else {
            sent = -1;

            break;
        }
    }

    return sent;
}

static int http_send(client_t *p_clt)
{
    int send_rslt = 0;

    ASSERT(NULL != p_clt);

    // 发送缓存
    send_rslt = send_buf(p_clt->m_cmnct_fd, &p_clt->m_send_buf);
    if ( -1 == send_rslt) {
        return -1;
    }
    ASSERT(send_rslt >= 0);
    p_clt->m_resp.m_sent_size += send_rslt;

    if (p_clt->m_send_buf.m_seek < p_clt->m_send_buf.m_size) {
        return 0;
    }

    if (p_clt->m_resp.m_sent_size >= p_clt->m_resp.m_total_size) {
        // 结束会话
        p_clt->m_select_type |= SELECT_MR;
        p_clt->m_select_type &= (~SELECT_MW);

        return 0;
    }

    // 继续填充缓存
    clean_buf(&p_clt->m_send_buf);
    switch (p_clt->m_resp.m_resp_status) {
    case RS_HTTP_200:
        {
            int read_size = 0;

            ASSERT(-1 != p_clt->m_resp.m_file.m_fd);
            read_size = read(p_clt->m_resp.m_file.m_fd,
                             p_clt->m_send_buf.mp_data,
                             MIN_BUF_SIZE);
            ASSERT(read_size > 0);
            p_clt->m_send_buf.m_seek = 0;
            p_clt->m_send_buf.m_size = read_size;

            break;
        }
    case RS_HTTP_403:
        {
            (void)memcpy(p_clt->m_send_buf.mp_data,
                         HTTP403CONTENT,
                         HTTP403CONTENT_LEN);
            p_clt->m_send_buf.m_seek = 0;
            p_clt->m_send_buf.m_size = HTTP403CONTENT_LEN;

            break;
        }
    case RS_HTTP_404:
        {
            (void)memcpy(p_clt->m_send_buf.mp_data,
                         HTTP404CONTENT,
                         HTTP404CONTENT_LEN);
            p_clt->m_send_buf.m_seek = 0;
            p_clt->m_send_buf.m_size = HTTP404CONTENT_LEN;

            break;
        }
    }

    return 0;
}

static int handle_http_requ(context_t *p_context, client_t *p_clt)
{
    int rdfd = 0;
    int requ_method = 0;
    str_t requ_line = {
        NULL, 0,
    };
    str_t item = {
        NULL, 0,
    };
    path_t path_tmp = {{NULL}};
    path_t filepath = {{NULL}};
    struct stat fp_stat = {0};
    char *p_filetype = NULL;

    ASSERT(NULL != p_context);
    ASSERT(NULL != p_clt);

    requ_line.mp_data = p_clt->m_recv_buf.mp_data;
    requ_line.m_len = 0;
    for (int i = 0; 0x00 != requ_line.mp_data[i]; ++i) {
        if ('\r' == requ_line.mp_data[i]) {
            break;
        }
        ++requ_line.m_len;
    }
    if (0 == requ_line.m_len) {
        fprintf(stderr, "[ERROR] illegal request!\n");

        return -1;
    }

    // ***** 解析请求行 *****
    // 解析请求方法
    item.mp_data = requ_line.mp_data;
    item.m_len = 0;
    for (int i = 0; '\r' != item.mp_data[i]; ++i) {
        if (SPACE == item.mp_data[i]) {
            break;
        }
        ++item.m_len;
    }
    requ_method = str_hash(item);
    switch (requ_method) {
    case RM_HTTP_GET:
        {
            p_clt->m_requ.m_requ_method = RM_HTTP_GET;

            break;
        }
    default:
        {
            fprintf(stderr, "[ERROR] unknown request method!\n");

            return -1;
        }
    }

    // 解析请求文件
    for (int i = 0; '\r' != item.mp_data[i]; ++i) {
        if ('/' == item.mp_data[i]) {
            item.mp_data += i;

            break;
        }
    }
    if ('/' != item.mp_data[0]) {
        fprintf(stderr, "[ERROR] parse request file failed!\n");

        return -1;
    }

    item.m_len = 0;
    for (int i = 0; '\r' != item.mp_data[i]; ++i) {
        if (SPACE == item.mp_data[i]) {
            break;
        }
        ++item.m_len;
    }
    if ((item.m_len < 1) || (item.m_len > 128)) {
        fprintf(stderr, "[ERROR] illegal file name!\n");

        return -1;
    }

    // 生成文件路径
    init_path(&path_tmp);
    init_path(&filepath);
    STR_CPY(path_tmp.m_path, path_tmp.m_path.m_len, p_context->m_path_root);
    STR_CPY(path_tmp.m_path, path_tmp.m_path.m_len, item);
    if (NULL == realpath(path_tmp.m_path.mp_data, filepath.m_path.mp_data)) {
        p_clt->m_resp.m_file.m_fd = -1;
        p_clt->m_resp.m_file.m_size = HTTP404CONTENT_LEN;
        p_clt->m_resp.m_resp_status = RS_HTTP_404;
        p_clt->m_resp.mpc_mime = "text/html";
        p_clt->m_resp.m_sent_size = 0;
        p_clt->m_resp.m_total_size = 0;

        return 0;
    }
    filepath.m_path.m_len = strlen(filepath.m_path.mp_data);
    ASSERT('/' != filepath.m_path.mp_data[filepath.m_path.m_len - 1]);
    ASSERT(0 == stat(filepath.m_path.mp_data, &fp_stat));
    if (S_ISDIR(fp_stat.st_mode)) {
        filepath.m_path.mp_data[filepath.m_path.m_len] = '/';
        ++filepath.m_path.m_len;
        STR_CPY(filepath.m_path, filepath.m_path.m_len, INDEX_FILE);
    }

    // 获取文件类型
    p_filetype = &filepath.m_path.mp_data[filepath.m_path.m_len - 1];
    while (p_filetype != filepath.m_path.mp_data) {
        if ('.' == p_filetype[0]) {
            ++p_filetype;

            break;
        }
        --p_filetype;
    }

    // 打开文件
    rdfd = open(filepath.m_path.mp_data, O_RDONLY);
    if (-1 == rdfd) {
        p_clt->m_resp.m_file.m_fd = -1;
        p_clt->m_resp.m_file.m_size = HTTP403CONTENT_LEN;
        p_clt->m_resp.m_resp_status = RS_HTTP_403;
        p_clt->m_resp.mpc_mime = "text/html";
        p_clt->m_resp.m_sent_size = 0;
        p_clt->m_resp.m_total_size = 0;

        return 0;
    }
    ASSERT(0 != rdfd);

    // 获取文件大小
    ASSERT(0 == fstat(rdfd, &fp_stat));

    // 生成响应
    p_clt->m_resp.m_resp_status = RS_HTTP_200;
    p_clt->m_resp.m_file.m_fd = rdfd;
    p_clt->m_resp.m_file.m_size = fp_stat.st_size;
    if (0 == strcmp(p_filetype, "html")) {
        p_clt->m_resp.mpc_mime = "text/html";
    } else if (0 == strcmp(p_filetype, "png")) {
        p_clt->m_resp.mpc_mime = "image/png";
    } else if (0 == strcmp(p_filetype, "jpg")) {
        p_clt->m_resp.mpc_mime = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "jpeg")) {
        p_clt->m_resp.mpc_mime = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "txt")) {
        p_clt->m_resp.mpc_mime = "text/plain";
    } else if (0 == strcmp(p_filetype, "flv")) {
        p_clt->m_resp.mpc_mime = "video/flv";
    } else if (0 == strcmp(p_filetype, "mp4")) {
        p_clt->m_resp.mpc_mime = "video/mp4";
    } else if(0 == strcmp(p_filetype, "avi")) {
        p_clt->m_resp.mpc_mime = "video/avi";
    } else if(0 == strcmp(p_filetype, "rmvb")) {
        p_clt->m_resp.mpc_mime = "video/rmvb";
    } else {
        p_clt->m_resp.mpc_mime = "application/octet-stream";
    }
    p_clt->m_resp.m_sent_size = 0;
    p_clt->m_resp.m_total_size = 0;

    return 0;
}

static void handle_http_resp(client_t *p_clt)
{
    ASSERT(NULL != p_clt);

    if (0 == p_clt->m_resp.m_total_size) { // 计算发送总大小
        int ntow = 0;
        char len_buf[32] = {0x00};
        str_t len = {len_buf, 0};
        char const *pc_status_line = NULL;

        // 文件长度字符串
        ASSERT(offset_to_str(p_clt->m_resp.m_file.m_size,
                             &len,
                             ARRAY_COUNT(len_buf)) > 0);

        // 清空发送缓存
        clean_buf(&p_clt->m_send_buf);
        switch (p_clt->m_resp.m_resp_status) {
        case RS_HTTP_200:
            {
                pc_status_line = HTTP200;

                break;
            }
        case RS_HTTP_403:
            {
                pc_status_line = HTTP403;

                break;
            }
        case RS_HTTP_404:
            {
                pc_status_line = HTTP404;

                break;
            }
        }
        ntow = snprintf(p_clt->m_send_buf.mp_data,
                        p_clt->m_send_buf.m_capacity,
                        "%s"
                        SERVER_NAME
                        "Content-Length: %s\r\n"
                        "Cache-Control: no-cache\r\n"
                        "Connection: keep-alive\r\n"
                        "Content-Type: %s\r\n\r\n",
                        pc_status_line,
                        len.mp_data,
                        p_clt->m_resp.mpc_mime);
        ASSERT(ntow < p_clt->m_send_buf.m_capacity);  // 不会产生过长的响应
        p_clt->m_send_buf.m_size = ntow;

        p_clt->m_resp.m_total_size = ntow + p_clt->m_resp.m_file.m_size;
    }
    ASSERT(p_clt->m_resp.m_total_size > 0);

    //
    http_send(p_clt);

    return;
}

static void ts_perror(char const *pc_msg, int error_no)
{
    char *p_desc = NULL;
    char err_buf[256] = {0x00};

    ASSERT(NULL != pc_msg);

    p_desc = strerror_r(error_no, err_buf, ARRAY_COUNT(err_buf));
    err_buf[ARRAY_COUNT(err_buf) - 1] = 0x00;
    fprintf(stderr, "[ERROR] [%d] %s: %s\n", error_no, pc_msg, p_desc);

    return;
}

static void handle_connection(context_t *p_context)
{
    int cmnct_fd = 0;
    client_t *p_clt = NULL;

    ASSERT(NULL != p_context);

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
    p_clt->m_resp.m_file.m_fd = -1;
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
        if (-1 == create_buf(&p_clt->m_send_buf, 2 * MIN_BUF_SIZE)) {
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
        if (-1 == shutdown(p_clt->m_cmnct_fd, SHUT_RDWR)) {
            ts_perror("shut down failed!", errno);
        }
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
    if (-1 != p_clt->m_resp.m_file.m_fd) {
        ASSERT(0 != p_clt->m_resp.m_file.m_fd);
        ASSERT(0 == close(p_clt->m_resp.m_file.m_fd));
        p_clt->m_resp.m_file.m_fd = -1;
    }
    p_clt->m_select_type = 0;
    (void)memset(&p_clt->m_clt_addr, 0, sizeof(struct sockaddr_in));
    destroy_buf(&p_clt->m_recv_buf);
    destroy_buf(&p_clt->m_send_buf);

    return;
}

static void handle_data_input(context_t *p_context, client_t *p_clt)
{
    ASSERT(NULL != p_clt);

    while (TRUE) {
        int recved_size = 0;
        buf_t *p_recv_buf = &p_clt->m_recv_buf;
        int recv_errno = 0;

        if (is_buf_full(&p_clt->m_recv_buf)) { // 缓冲满
            if (p_clt->m_recv_buf.m_size > 4096 * 4) {
                // 恶意请求
                handle_disconnection(p_context, p_clt, FALSE); // 主动断开

                break;
            }
            if (-1 == doublesize_buf(&p_clt->m_recv_buf, 2)) {
                handle_disconnection(p_context, p_clt, FALSE); // 主动断开

                break;
            }
        }

        recved_size = recv(p_clt->m_cmnct_fd,
                           &p_recv_buf->mp_data[p_recv_buf->m_size],
                           p_recv_buf->m_size - p_recv_buf->m_size - 1,
                           0);
        recv_errno = errno;
        if (recved_size > 0) {
            p_recv_buf->m_size += recved_size;

            continue;
        } else if ((-1 == recved_size) && (EAGAIN == recv_errno)) {
            int requ_rslt = 0;

            ASSERT(0x00 == p_recv_buf->mp_data[p_recv_buf->m_size]);

            requ_rslt = handle_http_requ(p_context, p_clt);
            if (0 == requ_rslt) {
                // 清空接收缓冲
                clean_buf(&p_clt->m_recv_buf);

                // 有写事件
                p_clt->m_select_type &= (~SELECT_MR); // 暂时屏蔽读事件
                p_clt->m_select_type |= SELECT_MW; // 写事件置位

                break;
            } else if (-1 == requ_rslt) {
                handle_disconnection(p_context, p_clt, FALSE); // 主动断开

                break;
            } else {
                ASSERT(0);
            }
        } else {
            handle_disconnection(p_context, p_clt, TRUE); // 被动断开

            break;
        }
    }

    return;
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
            handle_data_input(p_context, p_clt);
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

        handle_http_resp(p_clt);
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
    fprintf(stderr, "%p\n", p_context->mp_client_cache);
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

#define TEST 0
int main(int argc, char *argv[])
{
#if TEST
    char const *pc_status_line = NULL;
    char len_buf[32] = {0x00};
    str_t len = {len_buf};
    buf_t buf;

    create_buf(&buf, MIN_BUF_SIZE);
    doublesize_buf(&buf, 2);
    doublesize_buf(&buf, 2);
    destroy_buf(&buf);
    offset_to_str(18, &len, 32);

    printf("%d\n", ARRAY_COUNT(len_buf));
    printf("%s %d\n", len.mp_data, len.m_len);

    return 0;
#else
    str_t path_root = {NULL};

    if (1 == argc) {
        fprintf(stderr, "usage: shfs filename\n");

        return 0;
    }
    path_root.mp_data = argv[1];
    path_root.m_len = strlen(argv[1]);

    return shfs_main(path_root);
#endif
}
