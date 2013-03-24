#ifndef __LIST_H__
#define __LIST_H__


// 单链表
typedef intptr_t list_t;

static inline
void add_node(list_t **pp_list, list_t *p_node)
{
    list_t *p_tmp = NULL;

    ASSERT(NULL != pp_list);
    ASSERT(NULL != p_node);

    p_tmp = *pp_list;
    *p_node = (list_t)p_tmp;
    *pp_list = p_node;

    return;
}

static inline
void rm_node(list_t **pp_list, list_t *p_node)
{
    list_t **pp_curr = NULL;

    ASSERT(NULL != pp_list);
    ASSERT(NULL != p_node);

    pp_curr = pp_list;
    while (*pp_curr) {
        if (p_node == *pp_curr) {
            *pp_curr = (list_t *)*p_node;
            *p_node = 0;

            break;
        }

        pp_curr = (list_t **)*pp_curr;
    }

    return;
}
#endif // __LIST_H__
