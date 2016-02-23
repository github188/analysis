#include <stdio.h>


// 牛逼的开方取倒，神奇的0x5f375a86
float inv_sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x; // get bits for floating VALUE

    i = 0x5f375a86 - (i >> 1); // gives initial guess y0
    x = *(float*)&i; // convert bits BACK to float
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy

    return x;
}


int main(int argc, char *argv[])
{
    fprintf(stderr, "%f\n", 1 / inv_sqrt(2));
    fprintf(stderr, "%f\n", 1 / inv_sqrt(3));
    fprintf(stderr, "%f\n", 1 / inv_sqrt(100));
    fprintf(stderr, "%f\n", 1 / inv_sqrt(89));

    return 0;
}
