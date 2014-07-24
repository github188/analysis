#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef void (*ngx_pool_cleanup_pt)(void *data);
typedef struct ngx_pool_large_s ngx_pool_large_t;
typedef struct ngx_pool_data_s ngx_pool_data_t;
typedef struct ngx_pool_s ngx_pool_t;

struct ngx_pool_large_s {
    ngx_pool_large_t *next;
    void *alloc;
};

struct ngx_pool_data_s {
    uint8_t *last;
    uint8_t *end;
    ngx_pool_t *next;
    uintptr_t failed;
};


int main(int argc, char *argv[], char *env[])
{
    return EXIT_SUCCESS;
}
