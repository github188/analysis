#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static
void __gen_next__(char const *str,
                  intptr_t n,
                  intptr_t *next,
                  intptr_t m)
{
    intptr_t i = 0;
    intptr_t j = 1;

    assert(m >= n);

    next[i] = -1;
    next[j] = 0;
    while (j < n - 1) {
        if ((str[i] == str[j])) {
            next[j + 1] = i + 1;
            ++i;
            ++j;
        } else {
            i = next[i];
            if (-1 == i) {
                i = 0;
                next[++j] = 0;
            }
        }
    }
}

intptr_t kmp(const char* text, const char* pattern)
{
    intptr_t rslt;
    intptr_t text_len = strlen(text);
    intptr_t pattern_len = strlen(pattern);
    intptr_t *next = calloc(pattern_len, sizeof(intptr_t));

    memset(next, 0, pattern_len);

    __gen_next__(pattern, pattern_len, next, pattern_len);
#if 0
    for (intptr_t i = 0; i < pattern_len; ++i) {
        fprintf(stderr, "%ld: %ld\n", i, next[i]);
    }
#endif

    // do kmp
    do {
        intptr_t i = 0;
        intptr_t j = 0;

        while ((i < text_len) && (j < pattern_len)) {
            if ((-1 == j) || (text[i] == pattern[j])) {
                ++i;
                ++j;
            } else {
                j = next[j];
            }
        }

        if (j >= pattern_len) {
            rslt = i - pattern_len;
        } else {
            rslt = -1;
        }
    } while (0);

    free(next);

    return rslt;
}

int main(int argc, char *argv[])
{
    //char text[] = "mississippi";
    char text[] = "xxxafrxabcabdaabmississippi";
    char pattern[] = "abcabdaab";

    fprintf(stderr, "index: %ld\n", kmp(text, pattern));

    return 0;
}
