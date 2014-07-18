#include <stdio.h>
#include <errno.h>
#include <string.h>


int main(int argc, char *argv[])
{
    int i = 0;

    for (i = 0; i < (sys_nerr - 1); ++i) {
        (void)fprintf(stderr, "[INFO] %d: %s\n", i, strerror(i));
    }

    return 0;
}
