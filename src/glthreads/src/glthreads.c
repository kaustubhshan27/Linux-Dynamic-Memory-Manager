#include "glthreads.h"

static bool _glthread_check_empty(glthread_t *list)
{
    return (list->head == NULL ? true : false);
}

static void _glthread_insert_node(glthread_node_t *current_node, glthread_node_t *new_node)
{
    if (current_node->prev != NULL)
    {
        current_node->prev->next = new_node;
        new_node->prev = current_node->prev;
    }
    new_node->next = current_node;
    current_node->prev = new_node;
}

static void _glthread_remove(glthread_node_t *node)
{
    if (node->prev != NULL && node->next != NULL)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = NULL;
        node->prev = NULL;
    }
    else if (node->prev == NULL && node->next != NULL)
    {
        node->next->prev = NULL;
        node->next = NULL;
    }
    else if (node->next == NULL && node->prev != NULL)
    {
        node->prev->next = NULL;
        node->prev = NULL;
    }
}

void glthread_init(glthread_t *list)
{
    list->head = NULL;
}

void glthread_init_node(glthread_node_t *glthread_node)
{
    glthread_node->next = NULL;
    glthread_node->prev = NULL;
}

void glthread_add_node_at_head(glthread_t *list, glthread_node_t *node)
{
    glthread_init_node(node);
    if (!_glthread_check_empty(list))
    {
        _glthread_insert_node(list->head, node);
    }
    list->head = node;
}

void glthread_remove_node(glthread_t *list, glthread_node_t *node)
{
    if (list->head == node)
    {
        list->head = node->next;
    }
    _glthread_remove(node);
}

void glthread_delete(glthread_t *list)
{
    glthread_node_t *node = NULL;
    GLTHREAD_ITERATE_BEGIN(list, node)
    {
        glthread_remove_node(list, node);
    }
    GLTHREAD_ITERATE_END;
}

void glthread_priority_insert(glthread_t *list, glthread_node_t *gl_node, int8_t (*comp_fn)(void *, void *),
                              size_t offset)
{
    glthread_init_node(gl_node);

    if (_glthread_check_empty(list))
    {
        list->head = gl_node;
    }
    else
    {
        glthread_node_t *curr = NULL, *prev = NULL;

        GLTHREAD_ITERATE_BEGIN(list, curr)
        {
            if (comp_fn(GLTHREAD_BASEOF(gl_node, offset), GLTHREAD_BASEOF(curr, offset)) == -1)
            {
                _glthread_insert_node(curr, gl_node);
                return;
            }
            prev = curr;
        }
        GLTHREAD_ITERATE_END;

        /* insert node at the end */
        prev->next = gl_node;
    }
}
