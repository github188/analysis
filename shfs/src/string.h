#ifndef __STRING_H__
#define __STRING_H__

// 字符串
typedef struct {
    char *mp_data;
    int m_len;
} str_t;

#define STR_CPY(str_dst, dst_offset, str_src)   {\
            (void)memcpy(&str_dst.mp_data[dst_offset], \
                         str_src.mp_data, \
                         str_src.m_len);\
            str_dst.m_len = str_src.m_len + dst_offset;\
            str_dst.mp_data[str_dst.m_len] = '\0';\
        }

static inline
void reverse_str(str_t *p_target)
{
    int i = 0;
    int j = 0;

    ASSERT(NULL != p_target->mp_data);
    ASSERT(p_target->m_len > 0);

    for (i = 0, j = p_target->m_len - 1; i < j; ++i, --j) {
        char tmp = p_target->mp_data[i];

        p_target->mp_data[i] = p_target->mp_data[j];
        p_target->mp_data[j] = tmp;
    }
}

static inline
int offset_to_str(off_t value, str_t *p_target, int n)
{
    int real_len = 0;
    off_t value_tmp = value;

    ASSERT(NULL != p_target->mp_data);

    if (value < 0) {
        return -1;
    }

    p_target->m_len = 0;
    ASSERT(0 == real_len);
    while ((0 != value_tmp) && (p_target->m_len < n)) {
        off_t next_value = value_tmp / 10;

        p_target->mp_data[p_target->m_len++]
            = value_tmp - next_value * 10 + '0';
        value_tmp = next_value;
        ++real_len;
    }
    reverse_str(p_target);
    p_target->m_len = MIN(n, real_len);

    return real_len;
}

#endif // __STRING_H__
