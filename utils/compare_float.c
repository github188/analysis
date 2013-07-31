#include <stdio.h>
#include <math.h>


int float_equal(float a, float b, float abs_epsilon, float rel_epsilon)
{
    if (a == b) {
        return 1;
    }

    if (fabs(a - b) < abs_epsilon) {
        return 1;
    }

    if (fabs(a > b)) {
        return (fabs((a - b) / a) > rel_epsilon) ? 1 : 0;
    }

    return (fabs((a - b) / b) > rel_epsilon) ? 1 : 0;
}
