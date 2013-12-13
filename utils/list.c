#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define ARRAY_COUNT(array)  (sizeof(array) / sizeof(array[0]))
#define OFFSET_OF(s, m)     ((size_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
            ({\
                const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                (type *)((uint8_t *)p_mptr - OFFSET_OF(type, member));\
             })


void list_add(intptr_t *head, intptr_t *node)
{
    *node = *head;
    *head = (intptr_t)node;
}

void list_remove(intptr_t *node)
{
    intptr_t *p_del = (intptr_t *)*node;

    *node = *(intptr_t *)*node;
    *p_del = 0;
}

typedef struct {
    int __x__;
    intptr_t __node__;
} data_t;

int list_rm(intptr_t *head, int d)
{
    int rslt = -1;
    intptr_t *iter = head;

    while (NULL != iter) {
        data_t *data = CONTAINER_OF((intptr_t *)*iter, data_t, __node__);

        if (d == data->__x__) {
            list_remove(iter);
            rslt = 0;
            break;
        }
        iter = (intptr_t *)*iter;
    }

    return rslt;
}


int main(int argc, char *argv[])
{
#if 0
    intptr_t list = 0;
    data_t data[8];
    int i = 0;
    intptr_t iter = 0;

    for (i = 0; i < (intptr_t)ARRAY_COUNT(data); ++i) {
        data[i].__x__ = i;
        list_add(&list, &data[i].__node__);
    }

    assert(0 == list_rm(&list, 0));
    assert(0 == list_rm(&list, 7));

    iter = list;
    while (0 != iter) {
        data_t *data = CONTAINER_OF((intptr_t *)iter, data_t, __node__);

        (void)fprintf(stderr, "%d\n", data->__x__);
        iter = *(intptr_t *)iter;
    }
#endif
    intptr_t head = 0;
    intptr_t node = 0;

    (void)fprintf(stderr, "&head: %p, &node: %p\n", &head, &node);
    list_add(&head, &node);
    (void)fprintf(stderr, "head: %x, node: %x\n", head, node);
    list_remove(&head);
    (void)fprintf(stderr, "head: %x, node: %x\n", head, node);

    return 0;
}

