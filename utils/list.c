#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define FALSE (0)
#define TRUE (!FALSE)
#define SIZE_OF(x) ((intptr_t)sizeof(x))
#define ARRAY_COUNT(array)  ((intptr_t)(SIZE_OF(array) / SIZE_OF(array[0])))
#define OFFSET_OF(s, m)     ((size_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
            ({\
                const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                (type *)((uint8_t *)p_mptr - OFFSET_OF(type, member));\
             })

// bitmap
typedef struct {
    intptr_t *__matrix__;
    intptr_t __len__; // bytes
} bitmap_t;

#define INIT_BITMAP(p, l) ((bitmap_t){p, l})
#define EMPTY_BITMAP INIT_BITMAP(NULL, 0)

static inline
intptr_t bitmap_set(bitmap_t *bm, intptr_t pos, intptr_t set)
{
    intptr_t value = !!set;

    if (pos < 0) {
        return -1;
    } else if (pos < (8 * bm->__len__)) {
        intptr_t offset = (pos / 8);
        intptr_t shift = pos - (offset * 8);

        bm->__matrix__[offset] |= (value << shift);

        return 0;
    } else {
        return -1;
    }
}

static inline
intptr_t bitmap_is_set(bitmap_t *bm, intptr_t pos)
{
    if (pos < 0) {
        return FALSE;
    } else if (pos < (8 * bm->__len__)) {
        intptr_t offset = (pos / 8);
        intptr_t shift = pos - (offset * 8);

        return !!(bm->__matrix__[offset] & (1 << shift));
    } else {
        return FALSE;
    }
}

static inline
intptr_t bitmap_search_first(bitmap_t *bm, intptr_t set)
{
    intptr_t index;
    intptr_t value = (set ? 0 : (~0));

    for (index = 0; index < bm->__len__; ++index) {
        if (value == bm->__matrix__[index]) {
            continue;
        }

        for (intptr_t j = 0; TRUE; ++j) {
            if ((!!set) == (!((1 << j) & bm->__matrix__[index]))) {
                continue;
            }
            index = index * 8 + j;
            break;
        }
        break;
    }

    if (index >= 8 * bm->__len__) {
        index = -1;
    }

    return index;
}

static inline
void bitmap_clean(bitmap_t *bm)
{
    (void)memset(bm->__matrix__, 0, bm->__len__);

    return;
}


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

typedef struct {
    intptr_t __obj_size__;
    void *__segment__;
    int8_t *__cache__;
    intptr_t __cache_size__;
    bitmap_t __service_condition_bitmap__;
    arraydlist_t *__head__;
} dlist_t;

typedef struct {
    int8_t *__base__;
    intptr_t __offset__;
} dlist_iter_t;

static inline
void *dlist_iter_ref(dlist_iter_t *iter)
{
    return NULL;
}

static inline
void dlist_begin(dlist_t *dlist, dlist_iter_t *iter)
{

}

static inline
void dlist_end(dlist_t *dlist, dlist_iter_t *iter)
{

}

static inline
intptr_t dlist_next(dlist_t *dlist, intptr_t index)
{
    arraydlist_t *p = NULL;
    intptr_t shell_size = dlist->__obj_size__ + SIZE_OF(arraydlist_t);

    p = (arraydlist_t *)(
        dlist->__cache__ + shell_size * index + dlist->__obj_size__
    );

    return p->__next__;
}

intptr_t init_dlist(dlist_t *dlist, intptr_t obj_size)
{
    intptr_t bitmap_size = 0;
    intptr_t pack_size = obj_size + SIZE_OF(arraydlist_t);
    intptr_t total_size = 0;

    dlist->__obj_size__ = obj_size;
    dlist->__cache_size__ = 8 * SIZE_OF(intptr_t);
    bitmap_size = (dlist->__cache_size__ - 1) / 8 + 1;

    total_size = bitmap_size + pack_size * dlist->__cache_size__;
    dlist->__segment__ = malloc(total_size);
    (void)memset(dlist->__segment__, 0, total_size);

    dlist->__cache__ = (int8_t *)dlist->__segment__ + bitmap_size;
    dlist->__service_condition_bitmap__ = (bitmap_t){
        (intptr_t *)dlist->__segment__, bitmap_size,
    };
    dlist->__head__ = (arraydlist_t *)(
        (int8_t *)dlist->__segment__ + bitmap_size + obj_size
    );
    bitmap_set(&dlist->__service_condition_bitmap__, 0, 1);

    return 0;
}

static inline
intptr_t __dlist_search_cache__(dlist_t *dlist, intptr_t set)
{
    intptr_t index;

    index = bitmap_search_first(&dlist->__service_condition_bitmap__, 0);
    if ((-1 != index) && set) {
        bitmap_set(&dlist->__service_condition_bitmap__, index, set);
    }

    return index;
}

static inline
void __dlist_resize__(dlist_t *dlist)
{
    int8_t *new_cache = NULL;
    intptr_t new_cache_size = dlist->__cache_size__ * 2;
    intptr_t new_bitmap_size = (new_cache_size - 1) / 8 + 1;

    new_cache = (int8_t *)malloc(new_bitmap_size + new_cache_size);
    (void)memset(new_cache, 0, new_bitmap_size + new_cache_size);
    (void)memcpy(new_cache,
                 dlist->__service_condition_bitmap__.__matrix__,
                 dlist->__cache_size__);
    free(dlist->__service_condition_bitmap__.__matrix__);
    dlist->__cache__ = new_cache + new_bitmap_size;
    dlist->__service_condition_bitmap__ = (bitmap_t){
        (intptr_t *)new_cache, new_bitmap_size,
    };

    return;
}

static inline
void __dlist_add__(dlist_t *dlist, intptr_t index)
{
    arraydlist_t *prev;
    arraydlist_t *next;
    arraydlist_t *node;
    intptr_t shell_size = dlist->__obj_size__ + SIZE_OF(arraydlist_t);

    prev = dlist->__head__;
    next = (arraydlist_t *)(
        dlist->__cache__
        + shell_size * dlist->__head__->__next__
        + dlist->__obj_size__
    );
    node = (arraydlist_t *)(
        dlist->__cache__
        + shell_size * index
        + dlist->__obj_size__
    );
    arraydlist_add(prev, next, node, index);

    return;
}

intptr_t dlist_insert(dlist_t *dlist, void *obj)
{
    intptr_t index;
    intptr_t cache_obj_offset;

    index = __dlist_search_cache__(dlist, TRUE);
    if (-1 == index) {
        __dlist_resize__(dlist);
        index = __dlist_search_cache__(dlist, TRUE);
    }
    assert(index >= 0);
    cache_obj_offset = (dlist->__obj_size__ + SIZE_OF(arraydlist_t)) * index;
    (void)memcpy(&dlist->__cache__[cache_obj_offset],
                 obj,
                 dlist->__obj_size__);

    // add to dlist
    __dlist_add__(dlist, index);

    return 0;
}

void exit_dlist(dlist_t *dlist)
{
    free(dlist->__service_condition_bitmap__.__matrix__);

    return;
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
    intptr_t __x__;
    intptr_t __node__;
    arraydlist_t __dnode__;
} data_t;

void print_data(data_t data)
{
    (void)fprintf(stderr, "%d\n", data.__x__);

    return;
}

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

#if 0
    intptr_t i = 0;
    data_t data[8];

    print_data((data_t){9, 0, {0, 0}});
    memset(data, 0, SIZE_OF(data));
    arraylist_add(&data[0].__node__, &data[2].__node__, 2);
    arraylist_add(&data[2].__node__, &data[3].__node__, 3);
    arraylist_add(&data[3].__node__, &data[7].__node__, 7);
    arraylist_del(&data[0].__node__, &data[2].__node__);
    for (i = 0; i < (intptr_t)ARRAY_COUNT(data); ++i) {
        (void)fprintf(stderr, "data[%d] : %d\n", i, data[i].__node__);
    }

    memset(data, 0, SIZE_OF(data));
    arraydlist_add(&data[0].__dnode__,
                   &data[0].__dnode__,
                   &data[0].__dnode__,
                   0);
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

    /*arraydlist_del(&data[3].__dnode__,
                   &data[2].__dnode__,
                   &data[7].__dnode__);*/

    for (i = 0; i < (intptr_t)ARRAY_COUNT(data); ++i) {
        (void)fprintf(stderr,
                      "data[%d] : (%d, %d)\n",
                      i,
                      data[i].__dnode__.__prev__,
                      data[i].__dnode__.__next__);
    }

    return 0;
#endif

#if 1
    typedef struct {
        intptr_t a;
        intptr_t b;
    } mydata_t;

    dlist_t dlist;
    mydata_t x1 = {1, 2};
    mydata_t x2 = {3, 4};
    mydata_t x3 = {5, 6};

    (void)init_dlist(&dlist, SIZE_OF(mydata_t));
    dlist_insert(&dlist, &x2);
    dlist_insert(&dlist, &x3);
    dlist_insert(&dlist, &x1);
    for (intptr_t iter = dlist.__head__->__next__;
         iter != 0;
         iter = dlist_next(&dlist, iter))
    {
        // mydata_t *d = CONTAINER_OF(iter, mydata_t, __dnode__);
        intptr_t shell_size = dlist.__obj_size__ + SIZE_OF(arraydlist_t);

        mydata_t *d = (mydata_t *)(dlist.__cache__ + shell_size * iter);

        fprintf(stderr, "%d, %d\n", d->a, d->b);
    }
    exit_dlist(&dlist);

    return 0;
#endif
}
