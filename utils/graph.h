#ifndef __GRAPH_H__
#define __GRAPH_H__


#include <stdint.h>


static inline
void list_add(intptr_t **head, intptr_t *node)
{
    *node = (intptr_t)*head;
    *head = node;
}

static inline
void list_remove(intptr_t **node)
{
    intptr_t *p_del = *node;

    *node = (intptr_t *)**node;
    *p_del = 0;
}


typedef struct {
    intptr_t __vertex__;
    intptr_t __next__;
} vertex_t;
#endif // __GRAPH_H__
