#ifndef _GL_THREAD_
#define _GL_THREAD_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct glthread_node {
    struct glthread_node *next;
    struct glthread_node *prev;
} glthread_node_t;

typedef struct glthread {
    glthread_node_t *head;
    unsigned int offset;
} glthread_t;

/* initializes a glthread */
void glthread_init(glthread_t *list, uint32_t offset);

/* initializes a glthread node */
void glthread_init_node(glthread_node_t *glthread_node);

/* adds a node to a glthread */
void glthread_add_node(glthread_t *list, glthread_node_t *node);

/* removes a node from a glthread */
void glthread_remove_node(glthread_t *list, glthread_node_t *node);

/* deletes the entire glthread */
void glthread_delete(glthread_t *list);

#define OFFSETOF(struct_type, field_name)   \
    (uint32_t)(&((struct_type *)NULL)->field_name)

#define BASEOF(glthread_node, struct_type, field_name)   \
    (struct_type *)((uint8_t *)glthread_node - (OFFSETOF(struct_type, field_name)))

#define ITERATE_GLTHREAD_BEGIN(list_ptr, node)                                                   \
{                                                                                                \
    glthread_node_t *_glthread_ptr = NULL;                                                       \ 
    for(node = list_ptr->head; node != NULL; node = _glthread_ptr) {                             \
        _glthread_ptr = node->next;                                                              \
         
#define ITERATE_GLTHREAD_END    }}

#endif /* _GL_THREAD_ */
