#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ARRAY_COUNT(a)      (sizeof(a) / sizeof(a[0]))
#define INC_ZERO_BYTE(x)    \
            (!!~((((x & 0x7F7F7F7F) + 0x7F7F7F7F) | x) | 0x7F7F7F7F))
#define ABS(x)                  \
        (\
            ((x) ^ ((x) >> (sizeof(x) * 8 - 1))) \
                - ((x) >> (sizeof(x) * 8 - 1))\
        )

int str_len(char const *pc_str)
{
    int count = 0;

    if (NULL == pc_str) {
        return -1;
    }

    for (count = 0; '\0' != pc_str[count]; ++count) {
    }

    return count;
}

char *str_cpy_n(char *p_dest,
                char const *pc_src,
                int n,
                int overlap)
{
    int low_to_high = 0;
    int quotient = 0;
    int reminder_base = 0;
    int reminder = 0;
    int src_len = 0;

    if (NULL == p_dest) {
        return NULL;
    }
    if (NULL == pc_src) {
        return NULL;
    }

    low_to_high = 1; // 默认从低到高拷贝
    if (ABS(p_dest - pc_src) < n) {
        if (!overlap) {
            return NULL;
        } else if (p_dest < pc_src) {
            low_to_high = 1;
        } else {
            low_to_high = 0;
        }
    }

    // 计算需要拷贝的长度
    src_len = str_len(pc_src);
    --n;
    n = (n < src_len) ? n : src_len;
    quotient = n / sizeof(long);
    reminder_base = quotient * sizeof(long);
    reminder = n % sizeof(long);

    if (low_to_high) {
        int i = 0;
        long *p_dest_tmp = (long *)p_dest;
        long const *pc_src_tmp = (long const *)pc_src;

        for (i = 0; i < quotient; ++i) {
            p_dest_tmp[i] = pc_src_tmp[i];
        }
        if (reminder > 0) {
            for (i = reminder_base;
                 i < reminder_base + reminder;
                 ++i)
            {
                p_dest[i] = pc_src[i];
            }
        }
    } else {
        int i = 0;
        long *p_dest_tmp = (long *)p_dest;
        long const *pc_src_tmp = (long const *)pc_src;

        if (reminder > 0) {
            for (i = reminder_base + reminder - 1;
                 i >= reminder_base;
                 --i)
            {
                p_dest[i] = pc_src[i];
            }
        }
        for (i = quotient - 1; i >= 0; --i) {
            p_dest_tmp[i] = pc_src_tmp[i];
        }
    }
    p_dest[n] = '\0';

    return p_dest;
}


// nginx的实现
char *cpystrn(char *dst, char *src, size_t n)
{
    if (n == 0) {
        return dst;
    }

    while (--n) {
        *dst = *src;

        if (*dst == '\0') {
            return dst;
        }

        dst++;
        src++;
    }

    *dst = '\0';

    return dst;
}


int main(int argc, char *argv[])
{
#define DST_SIZE    35
    char const *pc_orig = "hello, world!";
    char src[DST_SIZE] = {};
    char dst1[DST_SIZE];
    char *dst2 = (char *)malloc(DST_SIZE);

    memcpy(src, pc_orig, 14);
    fprintf(stderr, "src:[%s]\n", src);
    if (NULL != str_cpy_n(src, src + 1, strlen(pc_orig) + 1, 1)) {
        fprintf(stderr, "dst:[%s]\n", src);
    }
    memcpy(src, pc_orig, 14);
    fprintf(stderr, "src:[%s]\n", src);
    if (NULL != str_cpy_n(src + 1, src, strlen(pc_orig), 1)) {
        fprintf(stderr, "dst:[%s]\n", src + 1);
    }
    memcpy(src, pc_orig, 14);
    fprintf(stderr, "src:[%s]\n", src);
    if (NULL != str_cpy_n(dst1, src, DST_SIZE, 1)) {
        fprintf(stderr, "dst:[%s]\n", dst1);
    }
    memcpy(src, pc_orig, 14);
    fprintf(stderr, "src:[%s]\n", src);
    if (NULL != str_cpy_n(dst2, dst1, DST_SIZE, 1)) {
        fprintf(stderr, "dst:[%s]\n", dst2);
    }
    memcpy(src, pc_orig, 14);
    fprintf(stderr, "src:[%s]\n", src);
    if (NULL != str_cpy_n(dst1, dst2, DST_SIZE, 1)) {
        fprintf(stderr, "dst:[%s]\n", dst1);
    }

    return 0;
}
