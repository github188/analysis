#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


#define ARRAY_COUNT(array)  ((intptr_t)(sizeof(array) / sizeof(array[0])))
#define OFFSET_OF(s, m)     ((size_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
            ({\
                const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                (type *)((uint8_t *)p_mptr - OFFSET_OF(type, member));\
             })

// arraydlist
typedef struct {
    intptr_t __prev__;
    intptr_t __next__;
} arraydlist_t;

void arraydlist_add(arraydlist_t *prev,
                    arraydlist_t *next,
                    arraydlist_t *node,
                    intptr_t node_index)
{
    node->__next__ = prev->__next__;
    node->__prev__ = next->__prev__;
    prev->__next__ = node_index;
    next->__prev__ = node_index;
}

void arraydlist_del(arraydlist_t *prev,
                    arraydlist_t *next,
                    arraydlist_t *node)
{
    prev->__next__ = node->__next__;
    next->__prev__ = node->__prev__;
    node->__next__ = 0;
    node->__prev__ = 0;
}

// arraylist
void arraylist_add(intptr_t *prev, intptr_t *node, intptr_t node_index)
{
    *node = *prev;
    *prev = node_index;
}

void arraylist_del(intptr_t *prev, intptr_t *node)
{
    *prev = *node;
    *node = 0;
}

// list
void list_add(intptr_t *prev, intptr_t *node)
{
    *node = *prev;
    *prev = (intptr_t)node;

    return;
}

void list_remove(intptr_t *prev)
{
    intptr_t *del = (intptr_t *)*prev;

    *prev = *del;
    *del = 0;

    return;
}

typedef struct {
    int __x__;
    intptr_t __node__;
    arraydlist_t __dnode__;
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

    for (i = 0; i < ARRAY_COUNT(data); ++i) {
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

    return 0;
#endif

#if 0
    intptr_t head = 0;
    intptr_t node = 0;

    (void)fprintf(stderr, "&head: %p, &node: %p\n", &head, &node);
    list_add(&head, &node);
    (void)fprintf(stderr, "head: %x, node: %x\n", head, node);
    list_remove(&head);
    (void)fprintf(stderr, "head: %x, node: %x\n", head, node);

    return 0;
#endif

#if 1
    intptr_t i = 0;
    data_t data[8];

    memset(data, 0, sizeof(data));
    arraylist_add(&data[0].__node__, &data[2].__node__, 2);
    arraylist_add(&data[2].__node__, &data[3].__node__, 3);
    arraylist_add(&data[3].__node__, &data[7].__node__, 7);
    arraylist_del(&data[0].__node__, &data[2].__node__);
    for (i = 0; i < (intptr_t)ARRAY_COUNT(data); ++i) {
        (void)fprintf(stderr, "data[%d] : %d\n", i, data[i].__node__);
    }

    memset(data, 0, sizeof(data));
    arraydlist_add(&data[0].__dnode__,
                   &data[0].__dnode__,
                   &data[2].__dnode__,
                   2);
    arraydlist_add(&data[0].__dnode__,
                   &data[2].__dnode__,
                   &data[3].__dnode__,
                   3);
    arraydlist_add(&data[3].__dnode__,
                   &data[2].__dnode__,
                   &data[7].__dnode__,
                   7);

    /* that's NOT right, broke the dlist
    arraydlist_add(&data[2].__dnode__,
                   &data[3].__dnode__,
                   &data[7].__dnode__,
                   7);*/
    for (i = 0; i < (intptr_t)ARRAY_COUNT(data); ++i) {
        (void)fprintf(stderr,
                      "data[%d] : (%d, %d)\n",
                      i,
                      data[i].__dnode__.__prev__,
                      data[i].__dnode__.__next__);
    }

    return 0;
#endif
}
