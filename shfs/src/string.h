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
        real_len = -1;
    } else if (0 == value) {
        p_target->mp_data[0] = '0';
        p_target->mp_data[1] = 0x00;
        p_target->m_len = 1;

        real_len = 1;
    } else {
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
    }

    return real_len;
}

int find_max_repeat(char const *pc_str, int_t pass_len)
{
    int max_len = 0; // 最大重复长度

    ASSERT(NULL != pc_str);
    ASSERT((1 < pass_len) && (pass_len < strlen(pc_str)));

    for (max_len = pass_len - 1; max_len > 0; --max_len) {
        #define LEFT            0
        #define RIGHT           1

        for (int_t iter[2] = {max_len - 1, pass_len - 1};
             iter[RIGHT] >= 0;
             --iter[LEFT], --iter[RIGHT])
        {
            if (pc_str[iter[LEFT]] != pc_str[iter[RIGHT]]) {
                break;
            }
        }

        #undef RIGHT
        #undef LEFT
    }

    return max_len;
}

int find_string_kmp(char const *pc_str,
                    int str_len,
                    char const *pc_sub,
                    int sub_len)
{
    int rslt = -1;

    ASSERT(NULL != pc_str);
    ASSERT(NULL != pc_sub);

    do {
        int i = 0;
        int j = 0;
        int const STR_LEN = str_len;
        int const SUB_LEN = sub_len;

        if (SUB_LEN <= 0) {
            break;
        }

        while((STR_LEN - i) > (SUB_LEN - j)) {
            if (SUB_LEN == j) {
                rslt = i - j;
                break;
            }

            if (pc_str[i] == pc_sub[j]) {
                ++i;
                ++j;
                continue;
            }

            if (j > 1) {
                j = find_max_repeat(pc_sub, j);
            } else {
                --j;
            }

            ++i;
        }
    } while (0);

    return rslt;
}


#endif // __STRING_H__
