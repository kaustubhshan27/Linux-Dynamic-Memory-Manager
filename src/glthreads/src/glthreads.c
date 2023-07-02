#include "glthreads.h"

/**
 * @brief Checks if a GLThread list is empty.
 *
 * This function checks if a GLThread list is empty by examining the head pointer of the list.
 *
 * @param list Pointer to the GLThread list to check.
 * @return true if the list is empty, false otherwise.
 */
static bool _glthread_check_empty(glthread_t *list)
{
    return (list->head == NULL ? true : false);
}

/**
 * @brief Inserts a GLThread node into a GLThread list.
 *
 * This function inserts a GLThread node into a GLThread list. It inserts the new node before
 * the current node in the list. The function updates the next and prev pointers of the
 * involved nodes to maintain the correct linkage.
 *
 * @param current_node Pointer to the current GLThread node in the list.
 * @param new_node Pointer to the new GLThread node to insert.
 */
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

/**
 * @brief Removes a GLThread node from a GLThread list.
 *
 * This function removes a GLThread node from a GLThread list. It updates the next and prev
 * pointers of the adjacent nodes to maintain the correct linkage after removal.
 *
 * @param node Pointer to the GLThread node to remove from the list.
 */
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

/**
 * @brief Initializes a GLThread list.
 *
 * This function initializes a GLThread list by setting the head pointer of the list to NULL.
 *
 * @param list Pointer to the GLThread list to initialize.
 */
void glthread_init(glthread_t *list)
{
    list->head = NULL;
}

/**
 * @brief Initializes a GLThread node.
 *
 * This function initializes a GLThread node by setting the next and prev pointers of the node to NULL.
 *
 * @param glthread_node Pointer to the GLThread node to initialize.
 */
void glthread_init_node(glthread_node_t *glthread_node)
{
    glthread_node->next = NULL;
    glthread_node->prev = NULL;
}

/**
 * @brief Adds a GLThread node at the head of a GLThread list.
 *
 * This function adds a GLThread node at the head of a GLThread list. It initializes the node,
 * checks if the list is empty, and inserts the node accordingly. Finally, it updates the head
 * pointer of the list to point to the newly added node.
 *
 * @param list Pointer to the GLThread list.
 * @param node Pointer to the GLThread node to add at the head.
 */
void glthread_add_node_at_head(glthread_t *list, glthread_node_t *node)
{
    glthread_init_node(node);
    if (!_glthread_check_empty(list))
    {
        _glthread_insert_node(list->head, node);
    }
    list->head = node;
}

/**
 * @brief Removes a GLThread node from a GLThread list.
 *
 * This function removes a GLThread node from a GLThread list. If the node to be removed is the head
 * of the list, the head pointer is updated to point to the next node. The function then calls the
 * `_glthread_remove` function to remove the node from the list completely.
 *
 * @param list Pointer to the GLThread list.
 * @param node Pointer to the GLThread node to remove.
 */
void glthread_remove_node(glthread_t *list, glthread_node_t *node)
{
    if (list->head == node)
    {
        list->head = node->next;
    }
    _glthread_remove(node);
}

/**
 * @brief Deletes a GLThread list and frees all its nodes.
 *
 * This function deletes a GLThread list and frees all its nodes. It iterates through the nodes in the list
 * using the GLTHREAD_ITERATE macro and calls the glthread_remove_node function to remove each node from the list.
 * Finally, the list itself is freed.
 *
 * @param list Pointer to the GLThread list to delete.
 */
void glthread_delete(glthread_t *list)
{
    glthread_node_t *node = NULL;
    GLTHREAD_ITERATE_BEGIN(list, node)
    {
        glthread_remove_node(list, node);
    }
    GLTHREAD_ITERATE_END;
}

/**
 * @brief Inserts a GLThread node with priority into a GLThread list.
 *
 * This function inserts a GLThread node with priority into a GLThread list. It initializes the node,
 * checks if the list is empty, and inserts the node at the appropriate position based on the provided
 * comparison function. The comparison function is used to determine the priority order. The offset
 * parameter is used to calculate the base address of the objects being compared.
 *
 * @param list Pointer to the GLThread list.
 * @param gl_node Pointer to the GLThread node to insert.
 * @param comp_fn Pointer to the comparison function for priority ordering.
 * @param offset Offset to calculate the base address of the objects being compared.
 */
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
