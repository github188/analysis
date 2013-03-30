#ifndef __BUF_H__
#define __BUF_H__


// 缓冲区
#define MIN_BUF_SIZE            4096

typedef struct {
    char *mp_data;
    int m_seek; // 缓冲指针
    int m_size;
    int m_capacity;
} buf_t;

static inline
int_t create_buf(buf_t *const THIS, ssize_t size)
{
    ASSERT(NULL != THIS);

    if (size < MIN_BUF_SIZE) {
        size = MIN_BUF_SIZE;
    }

    THIS->mp_data = (char *)malloc(size);
    if (NULL == THIS->mp_data) {
        return -1;
    }
    THIS->m_seek = -1;
    THIS->m_size = 0;
    THIS->m_capacity = size;

    return 0;
}

static inline
void clean_buf(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    THIS->m_seek = 0;
    THIS->m_size = 0;

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

    return (THIS->m_size < THIS->m_capacity - 1) ? FALSE : TRUE;
}

static inline
int doublesize_buf(buf_t *const THIS, int n)
{
    char *p_tmp = NULL;
    ssize_t tmp_size = n * THIS->m_capacity;

    ASSERT(NULL != THIS);
    ASSERT(n > 1);

    p_tmp = (char *)malloc(tmp_size);
    if (NULL == p_tmp) {
        return -1;
    }

    if (NULL != THIS->mp_data) {
        (void)memcpy(&p_tmp[0], &THIS->mp_data[0], THIS->m_size);

        free(THIS->mp_data);
    }
    THIS->mp_data = p_tmp;
    THIS->m_capacity = tmp_size;

    return 0;
}

static inline
void destroy_buf(buf_t *const THIS)
{
    ASSERT(NULL != THIS);

    free(THIS->mp_data);
    THIS->mp_data = NULL;
    THIS->m_size = 0;
    THIS->m_capacity = 0;
}
#endif // __BUF_H__
