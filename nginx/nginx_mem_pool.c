#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// 内存块最小大小
#define MIN_POOL_SIZE       4096


typedef intptr_t int_t;
typedef uintptr_t uint_t;

typedef void (*ngx_pool_cleanup_pt)(void *);
typedef struct ngx_pool_cleanup_s ngx_pool_cleanup_t;
typedef struct ngx_pool_large_s ngx_pool_large_t;
typedef struct ngx_pool_data_s ngx_pool_data_t;
typedef struct ngx_pool_s ngx_pool_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt handler;
    void *data;
    ngx_pool_cleanup_t *next;
};

struct ngx_pool_large_s {
    ngx_pool_large_t *next;
    void *alloc;
};

struct ngx_pool_data_s {
    int8_t *last;
    int8_t *end;
    ngx_pool_t *next; // 内存池链表
    int_t failed;
};

struct ngx_pool_s {
    int_t max_size; // 能容纳的数据大小
    ngx_pool_data_t data;
    ngx_pool_t *current; // 当前内存块
};


static void *ngx_palloc_block(ngx_pool_t *pool, int_t size)
{
    int8_t *p;
    int_t sizep;
    ngx_pool_t *iter;
    ngx_pool_t *new_pool;

    sizep = (int_t)(pool->data.end - (int8_t *)pool);
    p = (int8_t *)malloc(sizep);
    if (NULL == p) {
        (void)fprintf(stderr, "[ERROR] malloc() failed\n");

        return NULL;
    }

    new_pool = (ngx_pool_t *)p;
    new_pool->data.last = p + sizeof(ngx_pool_t) + size;
    new_pool->data.end = p + sizep;
    new_pool->data.next = NULL;
    new_pool->data.failed = 0;

    // 更新current值
    for (iter = pool->current; NULL != iter->data.next; iter = iter->data.next) {
        if (iter->data.failed > 4) { // 忽略可能分配不到的内存块
            // 倾向于使用新的内存块，而不是失败次数低的
            if (NULL == iter->data.next) {
                iter->current = new_pool;
            } else {
                iter->current = iter->data.next;
            }
        }

        // 调到这个函数已经表明所有内存块都分配失败了
        ++iter->data.failed;
    }

    // 挂上新内存块
    iter->data.next = new_pool;

    return p + sizeof(ngx_pool_t);
}


ngx_pool_t *ngx_create_pool(int_t size)
{
    int8_t *p;
    ngx_pool_t *pool;

    // 用户参数过滤
    if (size < MIN_POOL_SIZE) {
        size = MIN_POOL_SIZE;
    }
    if (size < 2 * sizeof(ngx_pool_t)) {
        (void)fprintf(stderr, "[BUG] you saw me that means a BUG!!!\n");

        return NULL;
    }

    // 分配内存
    p = (int8_t *)malloc(size);
    if (NULL == p) {
        (void)fprintf(stderr, "[ERROR] malloc() failed\n");

        return NULL;
    }

    // 初始化
    pool = (ngx_pool_t *)p;
    pool->data.last = p + sizeof(ngx_pool_t);
    pool->data.end = p + size;
    pool->data.next = NULL;
    pool->data.failed = 0;

    pool->max_size = size - sizeof(ngx_pool_t);

    pool->current = pool; // 当前内存池

    return pool;
}


void *ngx_palloc(ngx_pool_t *pool, int_t size)
{
    return NULL;
}


int main(int argc, char *argv[], char *env[])
{
    return EXIT_SUCCESS;
}
