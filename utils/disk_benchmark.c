#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>


#define BLOCK_SIZE      4096
#define FILE_SIZE_GB    (1024 * 1024 * 1024)
#define FILE_MASK       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


int main(int argc, char *argv[], char *env[])
{
    int fd, done_size;
    int done, iter, count;
    char buf[4096];
    char const *filename;

    done = 1;
    if (argc > 1) {
        filename = argv[1];
    } else {
        filename = "disk_benchmark.dbm";
    }

    fd = open(filename, O_CREAT | O_CLOEXEC | O_EXCL | O_RDWR, FILE_MASK);
    if (-1 == fd) {
        (void)fprintf(stderr, "[ERROR] open() failed(%d)\n", errno);
        return EXIT_FAILURE;
    }

    done_size = 0;
    count = FILE_SIZE_GB / BLOCK_SIZE;
    for (iter = 0; iter < count; ++iter) {
        ssize_t wsize;

        wsize = write(fd, buf, sizeof(buf));
        if (-1 == wsize) {
            done = 0;
            (void)fprintf(stderr, "[ERROR] write() failed(%d\n)", errno);
            break;
        }
        done_size += wsize;
        (void)fprintf(stderr, "\r%.1f%%",
                      done_size / (1.0f * FILE_SIZE_GB) * 100);
    }
    (void)fprintf(stderr, "\n");


    if (-1 == close(fd)) {
        (void)fprintf(stderr, "[ERROR] close() failed(%d)\n", errno);
    }

    return done ? EXIT_SUCCESS : EXIT_FAILURE;
}
