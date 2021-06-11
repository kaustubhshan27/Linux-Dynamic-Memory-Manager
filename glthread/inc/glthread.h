#ifndef _GL_THREADS_
#define _GL_THREADS_

#include <stdlib.h>
#include <stdbool.h>

typedef struct glthread_node {
    struct glthread_node *next;
    struct glthread_node *prev;
} glthread_node_t;

typedef struct glthread {
    glthread_node_t *head;
    unsigned int offset;
} glthread_t;

void glthread_init(glthread_t *list, unsigned int offset);
void glthread_init_node(glthread_node_t *glthread_node);
void glthread_add_node(glthread_t *list, glthread_node_t *node);
void glthread_remove_node(glthread_t *list, glthread_node_t *node);
void glthread_delete(glthread_t *list);
bool is_glthread_empty(glthread_t *list);

#define OFFSETOF(struct_type, field_name)   \
    (unsigned int)(&((struct_type *)NULL)->field_name)

#define BASEOF(glthread_node, struct_type, field_name)   \
    (struct_type *)(glthread_node - (OFFSETOF(struct_type, field_name)))

#define ITERATE_GLTHREAD_BEGIN(list_ptr, struct_type, field_name, base_ptr)             \
{                                                                                       \
    glthread_node_t *glthread_node = NULL, *next = NULL;                                \
    for(glthread_node = list_ptr->head; glthread_node != NULL; glthread_node = next) {  \
        next = glthread_node->next;                                                     \
        base_ptr = BASEOF(glthread_node, struct_type, field_name);
         
#define ITERATE_GLTHREAD_END    }}

#endif /* _GL_THREADS_ */
