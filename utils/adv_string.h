#ifndef __ADV_STRING__
#define __ADV_STRING__

#include <stdint.h>

typedef struct {
    char *__str__;
    intptr_t __len__;
} adv_string_t;
#define DEF_STRING(name, ptr, size) adv_string_t name = {ptr, size}

// kmp algorithm

// bm algorithm
static inline
void prepare_bm_bc(adv_string_t const *pattern,
                   intptr_t *bc,
                   intptr_t bc_size)
{
    for (intptr_t i = 0; i < bc_size; ++i) {
        bc[i] = pattern->__len__;
    }
    for (intptr_t i = 0; i < pattern->__len__ - 1; ++i) {
        bc[(intptr_t)pattern->__str__[i]] = pattern->__len__ - 1 - i;
    }
}

static inline
intptr_t adv_str_find(adv_string_t const *text, adv_string_t const *pattern)
{
    intptr_t rslt = 0;

    return rslt;
}
#endif // __ADV_STRING__
