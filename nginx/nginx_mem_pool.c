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

// ngx_pool_data_t才是内存块链
struct ngx_pool_data_s {
    int8_t *last;
    int8_t *end;
    ngx_pool_t *next;
    int_t failed;
};

struct ngx_pool_s {
    int_t max_size; // 能容纳的数据大小
    ngx_pool_data_t data; // 内存块链表头
    ngx_pool_t *current; // 当前内存池
};


static ngx_pool_t *ngx_create_pool(int_t size)
{
    int8_t *p;
    ngx_pool_t *pool;

    if (size < MIN_POOL_SIZE) {
        size = MIN_POOL_SIZE;
    }
    if (size < 2 * sizeof(ngx_pool_t)) {
        (void)fprintf(stderr, "you saw me that means a BUG!!!\n");
    }

    p = (int8_t *)malloc(size);
    pool = (ngx_pool_t *)p;

    pool->data.last = p + sizeof(ngx_pool_t);
    pool->data.end = p + size;
    pool->data.next = NULL;
    pool->data.failed = 0;

    pool->max_size = size - sizeof(ngx_pool_t);

    pool->current = pool; // 当前内存池

    return pool;
}


static void *ngx_palloc(ngx_pool_t *pool, int_t size)
{
    return NULL;
}


int main(int argc, char *argv[], char *env[])
{
    return EXIT_SUCCESS;
}
