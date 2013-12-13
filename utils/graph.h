#ifndef __GRAPH_H__
#define __GRAPH_H__


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rbtree.h"


static inline
void list_add(intptr_t *head, intptr_t *node)
{
    *node = *head;
    *head = (intptr_t)node;

    return;
}

static inline
void list_remove(intptr_t *node)
{
    intptr_t *p_del = (intptr_t *)*node;

    *node = *(intptr_t *)*node;
    *p_del = 0;

    return;
}


typedef struct s_vertex vertex_t;
struct s_vertex {
    vertex_t *__vertex__;
    intptr_t __weight__;
    struct rb_node __rb_node__;
    intptr_t __list_node__;
};

typedef struct {
    struct rb_root __vertexes__;
    intptr_t __vertex_count__;
    intptr_t __edge_count__;
} graph_t;
extern intptr_t init_graph(graph_t *graph);
extern void graph_add_vertex(graph_t *graph, vertex_t *vertex);
extern intptr_t graph_del_vertex(graph_t *graph, vertex_t *vertex);
extern void graph_add_edege(graph_t *graph, vertex_t *from, vertex_t *to);
extern intptr_t graph_del_edege(graph_t *graph,
                                vertex_t *from,
                                vertex_t *prev_of_to);
extern intptr_t graph_vertex_count(graph_t *graph);
extern intptr_t graph_edege_count(graph_t *graph);
extern void exit_graph(graph_t *gpaph);
#endif // __GRAPH_H__
