#include "graph.h"


#define OFFSET_OF(s, m)     ((size_t)&(((s *)0)->m ))
#define CONTAINER_OF(ptr, type, member)     \
            ({\
                const __typeof__(((type *)0)->member) *p_mptr = (ptr);\
                (type *)((uint8_t *)p_mptr - OFFSET_OF(type, member));\
            })


// graph
static intptr_t __search_vertex__(vertex_t *vertex,
                                  struct rb_node ***iter,
                                  struct rb_node **parent)
{
    int rslt = -1;

    while (NULL != **iter) {
        vertex_t *tmp_vertex = NULL;

        parent = *iter;
        tmp_vertex = CONTAINER_OF(*parent, vertex_t, __rb_node__);
        if (vertex < tmp_vertex) {
            *iter = &(*parent)->rb_left;
        } else if (vertex > tmp_vertex) {
            *iter = &(*parent)->rb_right;
        } else {
            rslt = 0;
            break;
        }
    }

    return rslt;
}

intptr_t init_graph(graph_t *graph)
{
    graph->__vertexes__ = RB_ROOT;
    graph->__vertex_count__ = 0;
    graph->__edge_count__ = 0;

    return 0;
}

intptr_t graph_add_vertex(graph_t *graph, vertex_t *vertex)
{
    intptr_t rslt = 0;
    struct rb_node *parent = NULL;
    struct rb_node **tmp = &graph->__vertexes__.rb_node;

    // add to rb_tree
    if (-1 == __search_vertex__(vertex, &tmp, &parent)) {
        rb_link_node(&vertex->__rb_node__, *tmp, tmp);
        rb_insert_color(&vertex->__rb_node__, &graph->__vertexes__);
        ++graph->__vertex_count__;
        rslt = 0;
    } else {
        rslt = -1;
    }

    return rslt;
}

intptr_t graph_del_vertex(graph_t *graph, vertex_t *vertex)
{
    intptr_t rslt = 0;
    struct rb_node *parent = NULL;
    struct rb_node **tmp = &graph->__vertexes__.rb_node;

    // add to rb_tree
    if (0 == __search_vertex__(vertex, &tmp, &parent)) {
        rb_erase(&vertex->__rb_node__, &graph->__vertexes__);
        --graph->__vertex_count__;
        rslt = 0;
    } else {
        rslt = -1;
    }


    return rslt;
}

intptr_t graph_add_edege(graph_t *graph, vertex_t *from, vertex_t *to)
{
    return 0;
}

intptr_t graph_del_edege(graph_t *graph, vertex_t *from, vertex_t *to)
{
    return 0;
}

intptr_t graph_vertex_count(graph_t *graph)
{
    return graph->__vertex_count__;
}

intptr_t graph_edege_count(graph_t *graph)
{
    return graph->__edge_count__;
}

void exit_graph(graph_t *gpaph)
{
    return;
}


// test
int main(int argc, char *argv[])
{
    int i = 0;
    int times = 1000;
    graph_t g;
    vertex_t *vts = (vertex_t *)calloc(times, sizeof(vertex_t));

    init_graph(&g);
    for (i = 0; i < times; ++i) {
        assert(0 == graph_add_vertex(&g, &vts[i]));
    }
    for (i = 0; i < times; ++i) {
        assert(0 == graph_del_vertex(&g, &vts[i]));
    }
    exit_graph(&g);

    return 0;
}
