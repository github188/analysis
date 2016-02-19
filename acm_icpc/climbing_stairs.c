#include <stdio.h>
#include <stdlib.h>


// version 1
/*
int climbStairs(int n) {
    if (n < 1) {
        return -1;
    }

    if (1 == n) {
        return 1;
    } else if (2 == n) {
        return 2;
    }

    return climbStairs(n - 1) + climbStairs(n -2);
}
*/

// version 2
int climbStairs(int n) {
    int *nn, i;
    if (n < 1) {
        return -1;
    }

    nn = (int *)calloc(n + 1, sizeof(int));
    nn[1] = 1;
    nn[2] = 2;

    for (i = 3; i < n + 1; ++i) {
        nn[i] = nn[i-1] + nn[i-2];
    }

    return nn[n];
}


int main(int argc, char *argv[]) {
    fprintf(stderr, "%d\n", climbStairs(1));
    fprintf(stderr, "%d\n", climbStairs(2));
    fprintf(stderr, "%d\n", climbStairs(3));
    fprintf(stderr, "%d\n", climbStairs(4));

    return 0;
}
