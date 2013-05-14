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
        mid = (start + stop) / 2;
        
        if (value == array[mid]) {
            rslt = 0;
            
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
    int xxx[] = {
      1, 5, 8, 23, 44, 97, 126,  
    };
    
    fprintf(stderr, "%d\n", binary_search(NULL, 0, 8));
    fprintf(stderr, "%d\n", binary_search(xxx, 0, 8));
    fprintf(stderr, "%d\n", binary_search(xxx, 1, 8));
    fprintf(stderr, "%d\n", binary_search(xxx, sizeof(xxx) / sizeof(xxx[0]), 1));
    fprintf(stderr, "%d\n", binary_search(xxx, sizeof(xxx) / sizeof(xxx[0]), 8));
    fprintf(stderr, "%d\n", binary_search(xxx, sizeof(xxx) / sizeof(xxx[0]), 126));
    fprintf(stderr, "%d\n", binary_search(xxx, sizeof(xxx) / sizeof(xxx[0]), 13));
}
//end //提示：自动阅卷结束唯一标识，请勿删除或增加。
