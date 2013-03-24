#ifndef __BUF_H__
#define __BUF_H__


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
#endif // __BUF_H__
