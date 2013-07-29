#include <stdio.h>


int *bsearch(int data[], int n, int x)
{
    int *p_rslt = NULL;
    int *p_start = data;
    int *p_mid = NULL;
    int *p_end = data + n;

    if ((NULL == data) || (n < 1)) {
        goto EXIT;
    }

    while (p_start < p_end) {
        p_mid = p_start + (p_end - p_start) / 2;

        if (*p_mid > x) {
            p_end = p_mid;
        } else if (*p_mid < x) {
            p_start = p_mid + 1;
        } else {
            p_rslt = p_mid;
            break;
        }
    }

EXIT:
    return p_rslt;
}

int main(void)
{
    int i = 0;
    int *px = NULL;
    int ax[] = {23, 56, 88, 109, 333, 456, 551, 767, 845};

    for (i = 0; i < (int)(sizeof(ax) / sizeof(ax[0])); ++i) {
        px = bsearch(ax, sizeof(ax) / sizeof(ax[0]), ax[i]);
        if (NULL == px) {
            continue;
        }
        fprintf(stderr, "%d\n", *px);
    }

    px = bsearch(ax, sizeof(ax)/sizeof(ax[0]), 22);
    if (NULL != px) {
        fprintf(stderr, "%d\n", *px);
    }
    px = bsearch(ax, sizeof(ax)/sizeof(ax[0]), 92);
    if (NULL != px) {
        fprintf(stderr, "%d\n", *px);
    }
    px = bsearch(ax, sizeof(ax)/sizeof(ax[0]), 522);
    if (NULL != px) {
        fprintf(stderr, "%d\n", *px);
    }
    px = bsearch(ax, sizeof(ax)/sizeof(ax[0]), 846);
    if (NULL != px) {
        fprintf(stderr, "%d\n", *px);
    }

    return 0;
}
