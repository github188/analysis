#include <stdio.h>


char *strcpyn(char *p_dest,
              char const *pc_src,
              int n,
              int overlap)
{
#define ABS(x)                  \
        (\
            ((x) ^ ((x) >> (sizeof(x) * 8 - 1))) \
                - ((x) >> (sizeof(x) * 8 - 1))\
        )
#define INC_ZERO_BYTE(x)    \
            (!!~((((x & 0x7F7F7F7F) + 0x7F7F7F7F) | x) | 0x7F7F7F7F))

    int low_to_high = 0;
    int quotient = n / sizeof(long);
    int reminder = n % sizeof(long);

    if (NULL == p_dest) {
        return NULL;
    }
    if (NULL == pc_src) {
        return NULL;
    }
    
    if (ABS(p_dest - pc_src) < n) {
        if (!overlap) {
            return NULL;
        } else if (p_dest < pc_src) {
            low_to_high = 1;
        } else {
            low_to_high = 0;
        }
    }

    if (low_to_high) {
        int i = 0;
        long *p_dest_tmp = (long *)p_dest;
        long const *pc_src_tmp = (long const *)pc_src;
        
        for (i = 0; i < quotient; ++i) {
            p_dest_tmp[i] = pc_src_tmp[i];
            if (INC_ZERO_BYTE(pc_src_tmp[i])) {
                break;
            }
        }
        if ((quotient == i) && (reminder > 0)) {
            for (i = 0; i < reminder; ++i) {
                p_dest[i] = pc_src[i];
                if ('\0' == pc_src[i]) {
                    break;
                }
            }
        }
    } else {
        int i = 0;
        long *p_dest_tmp = (long *)p_dest;
        long const *pc_src_tmp = (long const *)pc_src;
        
        for (i = quotient - 1; i >= 0; --i) {
            p_dest_tmp[i] = pc_src_tmp[i];
            if (INC_ZERO_BYTE(pc_src_tmp[i])) {
                break;
            }
        }
        if ((0 == i) && (reminder > 0)) {
            for (i = reminder - 1; i >= 0; --i) {
                p_dest[i] = pc_src[i];
                if ('\0' == pc_src[i]) {
                    break;
                }
            }
        }
    }

    return p_dest;

#undef INC_ZERO_BYTE
#undef ABS
}


int main(int argc, char *argv[])
{
    char dst[35] = {};
    char src[] = "hello, world!";

    if (NULL != strcpyn(dst, src, 35, 1)) {
        fprintf(stderr, "dst:[%s]\n", dst);
    }

    return 0;
}
