#include <stdio.h>


void qsort(int data[], size_t n)
{
    int i, j;
    size_t m = n / 2;

    if (1 == n) {
        return;
    }
    if (2 == n) {
        if (data[0] > data[1]) {
            int tmp;

            tmp = data[0];
            data[0] = data[1];
            data[1] = tmp;
        }

        return;
    }

    if (data[0] > data[n - 1]) {
        int tmp;

        if (data[m] > data[0]) {
            tmp = data[0];
            data[0] = data[n - 1];
            data[n - 1] = tmp;
        } else if (data[m] < data[n - 1]) {
        } else {
            tmp = data[m];
            data[m] = data[n - 1];
            data[n - 1] = tmp;
        }
    } else {
        int tmp;

        if (data[m] > data[n - 1]) {
        } else if (data[m] < data[0]) {
            tmp = data[0];
            data[0] = data[n - 1];
            data[n - 1] = tmp;
        } else {
            tmp = data[m];
            data[m] = data[n - 1];
            data[n - 1] = tmp;
        }
    }

    i = 0;
    j = n - 1 - 1;
    while (i <= j) {
        int tmp;

        if (data[i] < data[n - 1]) {
            ++i;
        } else {
            tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
            --j;
        }
    }

    fprintf(stderr, "%d, %d, %d\n", data[0], data[m], data[n - 1]);

    return;
}


int main(int argc, char *argv[], char *env[])
{
    int d[] = {1, 2, 3};
    int e[] = {1, 3, 2};
    int f[] = {2, 1, 3};
    int g[] = {2, 3, 1};
    int h[] = {3, 1, 2};
    int i[] = {3, 2, 1};
    qsort(d, 3);
    qsort(e, 3);
    qsort(f, 3);
    qsort(g, 3);
    qsort(h, 3);
    qsort(i, 3);

    return 0;
}
