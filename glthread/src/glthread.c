#include "../inc/glthread.h"

static void _glthread_add_next(glthread_node_t *current_node, glthread_node_t *new_node) {
    new_node->next = current_node;
    current_node->prev = new_node;
}

void glthread_init(glthread_t *list, unsigned int offset) {
    list->head = NULL;
    list->offset = offset;
}

void glthread_init_node(glthread_node_t *glthread_node) {
    glthread_node->next = NULL;
    glthread_node->prev = NULL;
}

void glthread_add_node(glthread_t *list, glthread_node_t *node) {
    glthread_init_node(node);
    if(is_glthread_empty(list)) {
        list->head = node;
    } else {
        glthread_node_t *head = list->head;
        _glthread_add_next(head, node);
        list->head = node;
    }
}

void glthread_remove_node(glthread_t *list, glthread_node_t *node) {

}

void glthread_delete(glthread_t *list) {

}

bool is_glthread_empty(glthread_t *list) {
    return ((list->head->next == NULL && list->head->prev == NULL) ? true:false);
}

