#ifndef _GL_THREAD_
#define _GL_THREAD_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct glthread_node
{
    struct glthread_node *next;
    struct glthread_node *prev;
} glthread_node_t;

typedef struct glthread
{
    struct glthread_node *head;
} glthread_t;

/* initializes a glthread */
void glthread_init(glthread_t *list);

/* initializes a glthread node */
void glthread_init_node(glthread_node_t *glthread_node);

/* adds a node to a glthread */
void glthread_add_node_at_head(glthread_t *list, glthread_node_t *node);

/* removes a node from a glthread */
void glthread_remove_node(glthread_t *list, glthread_node_t *node);

/* deletes the entire glthread */
void glthread_delete(glthread_t *list);

/* inserting a node into a priority queue */
void glthread_priority_insert(glthread_t *list, glthread_node_t *glthread, int8_t (*comp_fn)(void *, void *), size_t offset);

#define GLTHREAD_OFFSETOF(struct_type, field_name) (size_t)(&((struct_type *)NULL)->field_name)

#define GLTHREAD_BASEOF(glthread_node, offset)                                                        \
    (void *)((uint8_t *)glthread_node - offset)

#define GLTHREAD_ITERATE_BEGIN(list_ptr, node)                                                                         \
    {                                                                                                                  \
        glthread_node_t *_glthread_ptr = NULL;                                                                         \
        for (node = list_ptr->head; node != NULL; node = _glthread_ptr)                                                \
        {                                                                                                              \
            _glthread_ptr = node->next;

#define GLTHREAD_ITERATE_END                                                                                           \
    }                                                                                                                  \
    }

#endif /* _GL_THREAD_ */
