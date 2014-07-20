#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/sysctl.h>

extern char **environ;

static char **env;
static char *ngx_os_argv_last;

void __dump_mem__(void *p, intptr_t size)
{
    char *seg16;
    intptr_t segsize;

    seg16 = (char *)p;
    segsize = size;

    while (segsize > 0) {
        intptr_t max = (segsize > 16) ? 16 : segsize;

        (void)fprintf(stderr, "%p:", seg16);
        for (intptr_t i = 0; i < max; ++i) {
            (void)fprintf(stderr, " %02hhx", seg16[i]);
        }
        (void)fprintf(stderr, "\r\t\t\t\t\t\t;");
        for (intptr_t i = 0; i < max; ++i) {
            if ((seg16[i] > 32) && (seg16[i] < 127)) {
                (void)fprintf(stderr, "%c", seg16[i]);
            } else {
                (void)fprintf(stderr, " ");
            }
        }
        (void)fprintf(stderr, "\n");
        seg16 += 16;
        segsize -= 16;
    }

    return;
}


unsigned char *
ngx_cpystrn(unsigned char *dst, unsigned char *src, size_t n)
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

int ngx_init_setproctitle(char **ngx_os_argv)
{
    unsigned char      *p;
    size_t       size;
    unsigned int   i;

    size = 0;

    for (i = 0; env[i]; i++) {
        size += strlen(env[i]) + 1;
    }

    p = malloc(size);
    if (p == NULL) {
        return -1;
    }

    ngx_os_argv_last = ngx_os_argv[0];

    for (i = 0; ngx_os_argv[i]; i++) {
        if (ngx_os_argv_last == ngx_os_argv[i]) {
            ngx_os_argv_last = ngx_os_argv[i] + strlen(ngx_os_argv[i]) + 1;
        }
    }

    for (i = 0; env[i]; i++) {
        if (ngx_os_argv_last == env[i]) {

            size = strlen(env[i]) + 1;
            ngx_os_argv_last = env[i] + size;

            ngx_cpystrn(p, (unsigned char *) env[i], size);
            env[i] = (char *) p;
            p += size;
        }
    }

    ngx_os_argv_last--;

    return 0;
}

int main(int argc, char *argv[])
{
    int size = 0;
    env = environ;
    size = 0;

    for (int i = 0; env[i]; i++) {
        size += strlen(env[i]) + 1;
    }
    __dump_mem__(*env, size);

    ngx_init_setproctitle(argv);

    return 0;
}
