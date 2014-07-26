#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


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
    uint8_t *last;
    uint8_t *end;
    ngx_pool_t *next;
    uintptr_t failed;
};

struct ngx_pool_s {
    ngx_pool_data_t data; // 内存块链表头
};


static ngx_pool_t *ngx_create_pool(size_t size)
{
    return NULL;
}


int main(int argc, char *argv[], char *env[])
{
    return EXIT_SUCCESS;
}
