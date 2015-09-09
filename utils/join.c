#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MIN(a, b)               (((a) > (b)) ? (b) : (a))
#define REVERSE_BIT(a, ofst)    ((1U << (ofst)) ^ (a))

#define EXACT_NPARAMS           4
#define MIN_BITMAP_SIZE         268435456
#define MAX_ELEMENT     2000000000
#define JOIN_TYPE_INNER         "inner_join"
#define JOIN_TYPE_LEFT          "left_join"
#define JOIN_TYPE_RIGHT         "right_join"
#define JOIN_TYPE_OUTER         "outer_join"


typedef struct {
    char *data;
    size_t size;
} ju_str_t;
#define ju_str(data)            {data, sizeof(data) - 1}


// 打印内存
void dump_mem(void *p, intptr_t size)
{
    char *seg16;
    intptr_t segsize;

    seg16 = (char *)p;
    segsize = size;

    while (segsize > 0) {
        intptr_t max = (segsize > 16) ? 16 : segsize;

        fprintf(stderr, "%p:", seg16);
        for (intptr_t i = 0; i < max; ++i) {
            (void)fprintf(stderr, " %02hhx", seg16[i]);
        }
        (void)fprintf(stderr, "\n");
        seg16 += 16;
        segsize -= 16;
    }

    return;
}


static int ju_strncmp(ju_str_t *s1, ju_str_t *s2)
{
    return strncmp(s1->data, s2->data, MIN(s1->size, s2->size));
}


// 位图操作类型
enum {
    BITMAP_SET = 1,
    BITMAP_CLR,
    BITMAP_REVERSE,
};
static int handle_bitmap(ju_str_t *filename, ju_str_t *bm, int opt)
{
    FILE *fd;
    uint32_t elmt, line;
    char row[32] = {0};

    if (bm->size < MIN_BITMAP_SIZE) {
        abort();
    }

    fd = fopen(filename->data, "r");
    if (NULL == fd) {
        (void)fprintf(stderr, "[ERROR] open file %s failed:%d\n",
                      filename->data, ferror(fd));
        return -1;
    }

    line = 0;
    if (BITMAP_SET == opt) {
        while (! feof(fd)) {
            (void)fgets(row, sizeof(row), fd);
            ++line;
            elmt = (uint32_t)atoi(row);
            if (elmt > MAX_ELEMENT) {
                (void)fprintf(stderr, "[ERROR] out of range(%u):%u\n", line, elmt);
                continue;
            }
            bm->data[elmt / 8] |= (1U << (elmt & 7)); // 2^3-1=7，1U防止char高位负扩展
        }
    } else if (BITMAP_CLR == opt) {
        while (! feof(fd)) {
            (void)fgets(row, sizeof(row), fd);
            ++line;
            elmt = (uint32_t)atoi(row);
            if (elmt > MAX_ELEMENT) {
                (void)fprintf(stderr, "[ERROR] out of range(%u):%u\n", line, elmt);
                continue;
            }
            bm->data[elmt / 8] &= ~(1U << (elmt & 7)); // 2^3-1=7，1U防止char高位负扩展
        }
    } else if (BITMAP_REVERSE == opt) {
        while (! feof(fd)) {
            (void)fgets(row, sizeof(row), fd);
            ++line;
            elmt = (uint32_t)atoi(row);
            if (elmt > MAX_ELEMENT) {
                (void)fprintf(stderr, "[ERROR] out of range(%u):%u\n", line, elmt);
                continue;
            }
            bm->data[elmt / 8] = REVERSE_BIT(bm->data[elmt / 8], elmt & 7);
        }
    } else {
        abort();
    }

    (void)fclose(fd);

    return 0;
}

void dump_bm(ju_str_t *bm)
{
    for (int i = 0; i < bm->size; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (bm->data[i] & (1 << j)) {
                (void)fprintf(stdout, "%d\n", i * 8 + j);
            }
        }
    }

    return;
}


int main(int argc, char *argv[], char *env[])
{
    ju_str_t join_type_inner = ju_str(JOIN_TYPE_INNER);
    ju_str_t join_type_left = ju_str(JOIN_TYPE_LEFT);
    ju_str_t join_type_right = ju_str(JOIN_TYPE_RIGHT);
    ju_str_t join_type_outer = ju_str(JOIN_TYPE_OUTER);

    ju_str_t input_type;
    ju_str_t left_file;
    ju_str_t right_file;
    ju_str_t bm; // 栈内释放

    if (EXACT_NPARAMS != argc) {
        (void)fprintf(stderr, "usage: %s join_type file1 file2\n", argv[0]);
        (void)fprintf(stderr, "join_type - "
                              JOIN_TYPE_INNER ","
                              JOIN_TYPE_LEFT ","
                              JOIN_TYPE_RIGHT ","
                              JOIN_TYPE_OUTER "\n");
        return EXIT_FAILURE;
    }

    input_type.data = argv[1];
    input_type.size = strlen(argv[1]);
    left_file.data = argv[2];
    left_file.size = strlen(argv[2]);
    right_file.data = argv[3];
    right_file.size = strlen(argv[3]);

    bm.size = MIN_BITMAP_SIZE;
    bm.data = (char *)calloc(1, MIN_BITMAP_SIZE);

    if (0 == ju_strncmp(&join_type_inner, &input_type)) {
        // 交集
        if (handle_bitmap(&left_file, &bm, BITMAP_SET)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        if (handle_bitmap(&right_file, &bm, BITMAP_CLR)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        if (handle_bitmap(&left_file, &bm, BITMAP_REVERSE)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        dump_bm(&bm);
    } else if (0 == ju_strncmp(&join_type_left, &input_type)) {
        // 左文件差集
        if (handle_bitmap(&left_file, &bm, BITMAP_SET)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        if (handle_bitmap(&right_file, &bm, BITMAP_CLR)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        dump_bm(&bm);
    } else if (0 == ju_strncmp(&join_type_right, &input_type)) {
        // 右文件差集
        if (handle_bitmap(&right_file, &bm, BITMAP_SET)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        if (handle_bitmap(&left_file, &bm, BITMAP_CLR)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        dump_bm(&bm);
    } else if (0 == ju_strncmp(&join_type_outer, &input_type)) {
        // 并集
        if (handle_bitmap(&left_file, &bm, BITMAP_SET)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        if (handle_bitmap(&right_file, &bm, BITMAP_SET)) {
            free(bm.data);

            return EXIT_FAILURE;
        }

        dump_bm(&bm);
    } else {
        free(bm.data);
        (void)fprintf(stderr, "[ERROR] join type not supported right now\n");

        return EXIT_FAILURE;
    }

    free(bm.data);

    return EXIT_SUCCESS;
}
