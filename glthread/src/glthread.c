#include "../inc/glthread.h"

static bool _is_glthread_empty(glthread_t *list) {
    return ((list->head->next == NULL && list->head->prev == NULL) ? true:false);
}

static void _glthread_add_next(glthread_node_t *current_node, glthread_node_t *new_node) {
    new_node->next = current_node;
    current_node->prev = new_node;
}

static void _glthread_remove(glthread_node_t *node) {
    if(node->prev != NULL && node->next != NULL) {
        glthread_node_t *prev_node = node->prev;
        glthread_node_t *next_node = node->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        node->next = NULL;
        node->prev = NULL;
    } else {
        if(node->prev == NULL) {
            if(node->next != NULL) {
                glthread_node_t *next_node = node->next;
                next_node->prev = NULL;
                node->next = NULL;
            }
        }
        else if(node->next == NULL) {
            if(node->prev != NULL) {
                glthread_node_t *prev_node = node->prev;
                prev_node->next = NULL;
                node->prev = NULL;
            }
        }
    }
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
    if(_is_glthread_empty(list)) {
        list->head = node;
    } else {
        _glthread_add_next(list->head, node);
        list->head = node;
    }
}

void glthread_remove_node(glthread_t *list, glthread_node_t *node) {
    if(list->head == node) {
        list->head = node->next;
    }
    _glthread_remove(node);
}

void glthread_delete(glthread_t *list) {
    glthread_node_t *node;
    ITERATE_GLTHREAD_BEGIN(list, node) {
        glthread_remove_node(list, node);
    } ITERATE_GLTHREAD_END
}
