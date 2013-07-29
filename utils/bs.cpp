#include <stdio.h>
#include <iostream>
using namespace std;


int binary_search(int array[],int n,int value)
{
    //write your code here
    //找到了，返回找到的数值，没找到，返回-1
    int rslt = 0;
    int start = 0; // [start, stop)
    int stop = n;
    int mid = 0;

    if (NULL == array) {
        return -1;
    }
    if (n < 1) {
        return -1;
    }

    do {
        mid = start + (stop - start) / 2;

        if (value == array[mid]) {
            rslt = value;

            break;
        } else {
            rslt = -1;
            if (mid == start) {
                break;
            }
            if (value < array[mid]) {
                stop = mid;
            } else {
                start = mid + 1;
            }
        }
    } while (1);

    return rslt;
}


//start 提示：自动阅卷起始唯一标识，请勿删除或增加。
int main()
{
    //write your code here
    int rslt;
    int xxx[] = {23, 56, 88, 109, 333, 456, 551, 767, 845};

    for (int i = 0; i < (int)(sizeof(xxx) / sizeof(xxx[0])); ++i) {
        rslt = binary_search(xxx, sizeof(xxx)/sizeof(xxx[0]), xxx[i]);
        if (-1 == rslt) {
            continue;
        }
        fprintf(stderr, "%d\n", rslt);
    }
}
//end //提示：自动阅卷结束唯一标识，请勿删除或增加。
