/**
 * Given an array containing n distinct numbers taken from 0, 1, 2, ..., n, find the one that is missing from the array.
 * For example:
 *   Given nums = [0, 1, 3] return 2.
 * Note:
 *   Your algorithm should run in linear runtime complexity. Could you implement it using only constant extra space complexity?
 * Credits:
 *   Special thanks to @jianchao.li.fighter for adding this problem and creating all test cases.
 */
#include <stdio.h>
#include <assert.h>


// [0,n-1]连续数直接排序
void continuous_sort1(int *nums, int n)
{
    int i;

    for (i = 0; i < n; ++i) {
        nums[i] = i;
    }

    return;
}

// [0,n-1]连续数原地交换排序
void continuous_sort2(int *nums, int n)
{
    int i, j;
    int tmp;

    for (i = 0; i < n; ++i) {
        if (i == nums[i]) {
            continue;
        }

        j = i;
        while (nums[j] != j) {
            tmp = nums[nums[j]];
            nums[nums[j]] = nums[j];
            nums[j] = tmp;
            j = tmp;
        }
    }

    return;
}

int missingNumber(int *nums, int numsSize)
{
    int rslt, i;

    rslt = 0;
    for (i = 0; i < numsSize; ++i) {
        rslt ^= nums[i] ^ (i + 1);
    }

    return rslt;
}

void print_array(int *nums, int n)
{
    int i;

    for (i = 0; i < n; ++i) {
        fprintf(stderr, "%d,", nums[i]);
    }
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
    int x[] = {0, 1, 3, 4};
    fprintf(stderr, "%d\n", missingNumber(x, sizeof(x)/sizeof(x[0])));
    print_array(x, sizeof(x)/sizeof(x[0]));

    return 0;
}
