#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void dump_mem(void *p, int size)
{
    char *seg16;
    int segsize;

    seg16 = (char *)p;
    segsize = size;

    while (segsize > 0) {
        int i;
        int max = (segsize > 16) ? 16 : segsize;

        (void)fprintf(stdout, "%08X:", (unsigned int)(seg16 - (char *)p));
        for (i = 0; i < max; ++i) {
            (void)fprintf(stdout, " %02hhx", seg16[i]);
        }
        (void)fprintf(stdout, "\n");
        seg16 += 16;
        segsize -= 16;
    }

    return;
}


static int fake_main(char const *filename)
{
    int rslt;
    int fd;
    char buf[256];

    fd = open(filename, O_RDONLY);
    if (-1 == fd) {
        (void)fprintf(stderr, "open file %s failed\n", filename);
        rslt = -1;
        goto EXIT;
    }

    while (1) {
        ssize_t current_read_size;

        current_read_size = read(fd, buf, sizeof(buf));
        if (current_read_size <= 0) {
            break;
        }
        dump_mem(buf, current_read_size);
    }

    do {
        rslt = 0;
        (void)close(fd);
EXIT:
        break;
    } while (0);

    return rslt;
}


int main(int argc, char *argv[])
{
    int rslt;

    if (1 == argc) {
        (void)fprintf(stdout, "usage: %s filename\n", argv[0]);
        rslt = EXIT_SUCCESS;
    } else {
        rslt = ((0 == fake_main(argv[1])) ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    return rslt;
}
