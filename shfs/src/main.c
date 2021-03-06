// simple http server demo

#include "shfs.h"


#define DAEMON              FALSE
#define DEBUG               FALSE

#define LSN_PORT            8000
#define WORKER_PROCESSES    4
#define PAGE_SIZE           4096
#define SHM_MODE            0600
#define IPC_KEY             0xE78F8A

#define NODELAY         TRUE
#define REUSEADDR       TRUE
#define KEEPALIVE       TRUE

#define SPACE           0x20

extern int errno;
static int max_file_no = 0;
static int client_no = 0;
static int ipc_key = 0;
static uint32_t *sp_accept_lock = NULL;


// 信号处理
enum {
    SIG_NO_INT = 0xE7,
    SIG_NO_CHLD,
    SIG_NO_HUP,
    SIG_NO_PIPE,
};
static sig_atomic_t s_sig_no = 0;
static struct sigaction act = {};

static void handle_signal(int signo)
{
    switch (signo) {
    case SIGINT:
        {
            s_sig_no = SIG_NO_INT;

            break;
        }
    case SIGPIPE:
        {
            s_sig_no = SIG_NO_PIPE;

            break;
        }
    case SIGCHLD:
        {
            s_sig_no = SIG_NO_CHLD;

            break;
        }
    case SIGHUP:
        {
            s_sig_no = SIG_NO_HUP;

            break;
        }
    default:
        {
            break;
        }
    }
}

static struct {
    int m_signo;
    void (*m_sig_handler)(int);
} const s_signals[] = {
    // {SIGINT, &handle_signal},
    {SIGPIPE, &handle_signal},
    {SIGCHLD, &handle_signal},
    // {SIGHUP, &handle_signal},
};

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
    RS_HTTP_206 = 206,
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
    int m_requ_method; // 请求方法
    off_t m_range_b; // 起始偏移
} http_requ_t;

// http响应
typedef struct {
    int m_resp_status; // 响应状态
    file_t m_file;
    char const *mpc_mime; // mime类型
    off_t m_range_b; // 起始偏移
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

static void ts_perror(char const *pc_msg, int error_no);

static int fall_into_daemon(void)
{
    int fd = 0;

#define ESCAPE_BY_CRAFTY_SCHEME()       \
            switch (fork()) {\
            case -1:\
                {\
                    return -1;\
                }\
            case 0:\
                {\
                    break;\
                }\
            default:\
                {\
                    exit(0);\
                }\
            }

    ESCAPE_BY_CRAFTY_SCHEME();
    if (-1 == setsid()) {
        return -1;
    }
    ESCAPE_BY_CRAFTY_SCHEME();
#undef ESCAPE_BY_CRAFTY_SCHEME

    (void)umask(0);
    if (-1 == chdir("/")) {
        return -1;
    }

    for (int i = 0; i < max_file_no; ++i) {
        (void)close(i);
    }

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return -1;
    }
    if (-1 == dup2(fd, STDIN_FILENO)) {
        return -1;
    }

    if (-1 == dup2(fd, STDOUT_FILENO)) {
        return -1;
    }

    if (-1 == dup2(fd, STDERR_FILENO)) {
        return -1;
    }

    return 0;
}

static uint32_t atomic_cmp_set(uint32_t *lock, uint32_t old, uint32_t set)
{
    uint8_t rslt = 0;

    assert(NULL != lock);
    __asm__ __volatile__ ("lock;"
                          "cmpxchgl %3, %1;"
                          "sete %0;"
                          : "=a" (rslt)
                          : "m" (*lock), "a" (old), "r" (set)
                          : "cc", "memory");

    return rslt;
}

static int handle_accept(int lsn_fd, client_t *p_clt)
{
    int rslt = 0;
    int accept_errno = 0;
    socklen_t addrlen = 0;

    ASSERT(NULL != p_clt);

    if ((0 != *sp_accept_lock)
            || (!atomic_cmp_set(sp_accept_lock, 0, 1)))
    {
        return -1;
    }
    while (TRUE) {
        errno = 0;
        rslt = accept(lsn_fd,
                      (struct sockaddr *)&p_clt->m_clt_addr,
                      &addrlen);
        accept_errno = errno;
        if (rslt > 0) {
            break;
        }
        if ((ENOSYS == accept_errno) || (ECONNABORTED == accept_errno)) {
            ts_perror("accept failed", accept_errno);
            break;
        }
        if ((EAGAIN == accept_errno) || (EWOULDBLOCK == accept_errno)) {
            continue;
        }
    }
    *sp_accept_lock = 0;

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
    int worker_processes;
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
        errno = 0;

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
                             p_clt->m_send_buf.m_capacity);
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
    int range_b = 0;

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
    for (int i = 0;
         ('\r' != item.mp_data[i]) && (0x00 != item.mp_data[i]);
         ++i)
    {
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

    // 解析range行
    p_clt->m_requ.m_range_b = 0;
    range_b = find_string_kmp(requ_line.mp_data,
                              requ_line.m_len,
                              "RANGE",
                              sizeof("RANGE") - 1);
    if (-1 == range_b) {
        range_b = find_string_kmp(requ_line.mp_data,
                                  requ_line.m_len,
                                  "Range",
                                  sizeof("Range") - 1);
    }
    if (range_b >= 0) { // 断点续传
        str_t offset = {};

        range_b = find_string_kmp(requ_line.mp_data,
                                  requ_line.m_len,
                                  "bytes",
                                  sizeof("bytes") - 1);
        if (range_b < 0) { // 非法请求
            return -1;
        }
        while ((requ_line.mp_data[range_b] < '0')
                   || (requ_line.mp_data[range_b] > '9'))
        {
            if (0x00 == requ_line.mp_data[range_b]) {
                return -1;
            }
            ++range_b;
        }
        offset.mp_data = &requ_line.mp_data[range_b];
        offset.m_len = 0;
        for (int i = 0; 0x00 == offset.mp_data[i]; ++i) {
            if ('-' == offset.mp_data[i]) {
                break;
            }
            ++offset.m_len;
        }
        p_clt->m_requ.m_range_b = str_to_offset(&offset);
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
    } else if (0 == strcmp(p_filetype, "ico")) {
        p_clt->m_resp.mpc_mime = "image/x-icon";
    } else if (0 == strcmp(p_filetype, "png")) {
        p_clt->m_resp.mpc_mime = "image/png";
    } else if (0 == strcmp(p_filetype, "jpg")) {
        p_clt->m_resp.mpc_mime = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "jpeg")) {
        p_clt->m_resp.mpc_mime = "image/jpeg";
    } else if (0 == strcmp(p_filetype, "txt")) {
        p_clt->m_resp.mpc_mime = "text/plain";
    } else if (0 == strcmp(p_filetype, "mp3")) {
        p_clt->m_resp.mpc_mime = "audio/mpeg";
    } else if (0 == strcmp(p_filetype, "flv")) {
        p_clt->m_resp.mpc_mime = "video/flv";
    } else if (0 == strcmp(p_filetype, "mp4")) {
        p_clt->m_resp.mpc_mime = "video/mp4";
    } else if(0 == strcmp(p_filetype, "avi")) {
        p_clt->m_resp.mpc_mime = "video/avi";
    } else if(0 == strcmp(p_filetype, "rmvb")) {
        p_clt->m_resp.mpc_mime = "video/rmvb";
    } else if(0 == strcmp(p_filetype, "mkv")) {
        p_clt->m_resp.mpc_mime = "video/mkv";
    } else {
        p_clt->m_resp.mpc_mime = "application/octet-stream";
    }
    p_clt->m_resp.m_range_b = p_clt->m_requ.m_range_b;
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
                        "Content-Transfer-Encoding: binary\r\n"
                        "Cache-Control: must-revalidate, "
                            "post-check=0, pre-check=0\r\n"
                        "Connection: keep-alive\r\n"
                        "Pragma: public\r\n"
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

void ts_perror(char const *pc_msg, int error_no)
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
    int recv_buf_size = 0;
    int send_buf_size = 0;
    socklen_t option_len = sizeof(int);
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

    // 获取缓冲大小
    if (-1 == getsockopt(cmnct_fd,
                         SOL_SOCKET,
                         SO_RCVBUF,
                         &recv_buf_size,
                         &option_len))
    {
        ASSERT(0 == close(cmnct_fd));

        return;
    }

    if (-1 == getsockopt(cmnct_fd,
                         SOL_SOCKET,
                         SO_SNDBUF,
                         &send_buf_size,
                         &option_len))
    {
        ASSERT(0 == close(cmnct_fd));

        return;
    }

    // 初始化客户端
    p_clt->m_cmnct_fd = cmnct_fd;
    p_clt->m_resp.m_file.m_fd = -1;
    p_clt->m_select_type |= SELECT_MR; // 监视读事件
    if (is_buf_empty(&p_clt->m_recv_buf)) {
        if (-1 == create_buf(&p_clt->m_recv_buf, recv_buf_size)) {
            ASSERT(0 == close(cmnct_fd));

            return;
        }
    } else {
        clean_buf(&p_clt->m_recv_buf);
    }
    if (is_buf_empty(&p_clt->m_send_buf)) {
        if (-1 == create_buf(&p_clt->m_send_buf, send_buf_size)) {
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
            ts_perror("shut down failed", errno);
        }
    } else { // 主动断开
        if (-1 == shutdown(p_clt->m_cmnct_fd, SHUT_WR)) {
            ts_perror("shut down failed", errno);
        }
        while (TRUE) {
            char buf_tmp[256] = {0x00};

            if (recv(p_clt->m_cmnct_fd,
                     buf_tmp,
                     ARRAY_COUNT(buf_tmp),
                     0) > 0)
            {
                continue;
            } else {
                if (-1 == shutdown(p_clt->m_cmnct_fd, SHUT_RD)) {
                    ts_perror("shut down failed", errno);
                }

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

        errno = 0;
        recved_size = recv(p_clt->m_cmnct_fd,
                           &p_recv_buf->mp_data[p_recv_buf->m_size],
                           p_recv_buf->m_capacity - p_recv_buf->m_size - 1,
                           0);
        recv_errno = errno;

        if (recved_size > 0) {
            p_recv_buf->m_size += recved_size;

            continue;
        } else if ((-1 == recved_size) && (EAGAIN == recv_errno)) {
            int requ_rslt = 0;

            p_recv_buf->mp_data[p_recv_buf->m_size] = 0x00;
            #if DEBUG
                fprintf(stderr, "%s\n", p_recv_buf->mp_data);
            #endif
            requ_rslt = handle_http_requ(p_context, p_clt);
            if (0 == requ_rslt) {
                // 清空接收缓冲
                clean_buf(&p_clt->m_recv_buf);

                // 有写事件
                p_clt->m_select_type |= SELECT_MW;

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
            client_t *p_clt_tmp = CONTAINER_OF(p_iter, client_t, m_node);

            if (fd == p_clt_tmp->m_cmnct_fd) {
                p_clt = p_clt_tmp;

                break;
            }
        }

        if (NULL == p_clt) { // 连接已断开，资源也已释放
            break;
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

    if (-1 == listen(p_context->m_lsn_fd, SOMAXCONN)) {
        return -1;
    }

    while (TRUE) {
        int select_errno = 0;

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
        errno = 0;
        nevents = select(fd_max + 1, &fds_r, &fds_w, NULL, &io_wait_tv);
        select_errno = errno;

        if (nevents > 0) {
            handle_events(p_context, &fds_r, &fds_w, fd_max);
        } else if (0 == nevents) { // 超时
            continue;
        } else if (EINTR == select_errno) { // 慢系统调用被信号中断
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
    int nodelay = NODELAY;
    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(LSN_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_zero = {0},
    };
    int tmp_errno = 0;

    // 非阻塞io和FD_CLOEXEC
    if (-1 == fcntl(lsn_fd,
                    F_SETFL,
                    fcntl(lsn_fd, F_GETFL) | O_NONBLOCK | FD_CLOEXEC))
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
        return -1;
    }

    // TCP_NODELAY
    if (-1 == setsockopt(lsn_fd,
                         IPPROTO_TCP,
                         TCP_NODELAY, &nodelay, sizeof(nodelay)))
    {
        return -1;
    }

    // 绑定ip端口
    errno = 0;
    if (-1 == bind(lsn_fd,
                   (struct sockaddr *)&srv_addr,
                   sizeof(srv_addr)))
    {
        tmp_errno = errno;

        ts_perror("bind failed", tmp_errno);

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
    // sigset_t set = {};
    int tmp_errno = 0;

    // 获得能打开的最大描述符数目
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlmt)) {
        ASSERT(0 == close(lsn_fd));

        return -1;
    }
    max_file_no = rlmt.rlim_cur;

    // 初始化信号处理
    sigfillset(&act.sa_mask);
    for (int i = 0; i < ARRAY_COUNT(s_signals); ++i) {
        act.sa_handler = s_signals[i].m_sig_handler;
        act.sa_flags = 0;

        errno = 0;
        if (-1 == sigaction(s_signals[i].m_signo, &act, NULL)) {
            tmp_errno = errno;
            ts_perror("sigaction failed", tmp_errno);

            return -1;
        }
    }

    // 守护进程
    if ((DAEMON) && (-1 == fall_into_daemon())) {
        return -1;
    }

    // 创建共享内存
    errno = 0;
    ipc_key = shmget(IPC_PRIVATE,
                     PAGE_SIZE,
                     SHM_MODE | IPC_CREAT | IPC_EXCL);
    tmp_errno = errno;
    if (-1 == ipc_key) {
        if (EEXIST != tmp_errno) {
            return -1;
        }
    }

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

    // 申请客户端结构缓存
    client_no = max_file_no - max_file_no / 8;
    p_client_cache = calloc(client_no, sizeof(client_t));

    if (NULL == p_client_cache) {
        ASSERT(0 == close(lsn_fd));

        return -1;
    }

    // 建立客户端空闲链
    for (int i = 0; i < client_no; ++i) {
        add_node(&p_free_clients,
                 &p_client_cache[i].m_node);
    }

    // 填充运行时上下文
    p_context->worker_processes = WORKER_PROCESSES;
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

    // 销毁共享内存
    fprintf(stderr, "destroy shm\n");
    (void)shmctl(IPC_PRIVATE, IPC_RMID, 0);

    return;
}

static int slave_main(context_t *p_context)
{
    int rslt = 0;

    // 事件循环
    sp_accept_lock
        = (uint32_t *)((byte_t *)shmat(ipc_key, 0, 0) + 0);
    if ((uint32_t *)(~0) == sp_accept_lock) {
        fprintf(stderr, "[ERROR] shmat() failed: %d\n", errno);

        return -1;
    }
    *sp_accept_lock = 0;

    rslt = event_loop(p_context);

    ASSERT(0 == shmdt(sp_accept_lock));

    return rslt;
}

static int shfs_main(str_t const PATH_ROOT)
{
    int rslt = 0;
    int tmp_errno = 0;
    int exit_status = 0;
    context_t context = {0};

    // 创建运行上下文
    if (-1 == build_context(&context, PATH_ROOT)) {
        return -1;
    }

    do {
        pid_t pid = 0;
        sigset_t mask = {};

        for (int i = 0; i < context.worker_processes; ++i) {
            errno = 0;
            pid = fork();
            tmp_errno = errno;
            if (-1 == pid) {
                rslt = -1;
                ts_perror("fork failed", tmp_errno);

                break;
            } else if (0 == pid) {
                return slave_main(&context);
            } else {
                continue;
            }
        }

        if (-1 == rslt) {
            break;
        }

        // 等待信号
        sigemptyset(&mask);
        while (TRUE) {
            (void)sigsuspend(&mask);
            switch (s_sig_no) {
            case SIG_NO_CHLD:
                {
                    errno = 0;
                    pid = wait(&exit_status);
                    tmp_errno = errno;
                    fprintf(stderr,
                            "worker process %d exit with code %d\n",
                            pid,
                            WIFEXITED(exit_status)
                                ? WEXITSTATUS(exit_status) : 0xe78f8a);

                    break;
                }
            case SIG_NO_PIPE:
                {
                    fprintf(stderr, "[WARNING] sig pipe signal \n");
                }
            default:
                {
                    break;
                }
            }
        }
    } while (0);

    // 销毁运行上下文
    destroy_context(&context);

    return 0;
}


#define TEST 0
#define CORE_DUMP_TEST 0
int main(int argc, char *argv[])
{
#if TEST
    char const *pc_status_line = NULL;
    char len_buf[32] = {0x00};
    str_t len = {len_buf};
    buf_t buf;

#if CORE_DUMP_TEST
    int *p = NULL;

    *p = 13;
#endif

    create_buf(&buf, MIN_BUF_SIZE);
    doublesize_buf(&buf, 2);
    doublesize_buf(&buf, 2);
    destroy_buf(&buf);
    offset_to_str(18, &len, 32);

    printf("%d\n", ARRAY_COUNT(len_buf));
    printf("%s %d\n", len.mp_data, len.m_len);

    return 0;
#else
    int stat_errno = 0;
    path_t path_root = {};
    str_t path_root_tmp = {};
    struct stat root_stat = {};

    if (1 == argc) {
        fprintf(stderr, "usage: shfs filename\n");

        return 0;
    }

    errno = 0;
    if (-1 == stat(argv[1], &root_stat)) {
        stat_errno = errno;

        ts_perror("path dose NOT exist", stat_errno);

        return -1;
    }

    if (!S_ISDIR(root_stat.st_mode)) {
        fprintf(stderr, "[ERROR] path is NOT a directory!\n");

        return -1;
    }

    init_path(&path_root);
    path_root_tmp.mp_data = argv[1];
    path_root_tmp.m_len = strlen(argv[1]);
    if (NULL == realpath(path_root_tmp.mp_data,
                         path_root.m_path.mp_data))
    {
        return -1;
    }
    path_root.m_path.m_len = strlen(path_root.m_path.mp_data);

    return shfs_main(path_root.m_path);
#endif
}
