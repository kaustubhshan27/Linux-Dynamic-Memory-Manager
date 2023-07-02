#include "mm.h"

/* size of a VM page on this system */
static size_t SYSTEM_PAGE_SIZE = 0;

/* pointer to the current head of the VM page lists containing struct records */
static vm_page_for_struct_records_t *vm_page_record_head = NULL;

/**
 * @brief Requests a virtual memory page.
 *
 * This function requests a virtual memory page by mapping it into the process's address space.
 *
 * @param units Number of units (pages) to request.
 * @return Pointer to the requested virtual memory page, or NULL if the request failed.
 */
static void *_mm_request_vm_page(uint32_t units)
{
    /* the virtual mapping should be done in the heap */
    uint8_t *vm_page =
        mmap(sbrk(0), units * SYSTEM_PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (vm_page == MAP_FAILED)
    {
        return NULL;
    }
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);

    return (void *)vm_page;
}

/**
 * @brief Releases a virtual memory page.
 *
 * This function releases a virtual memory page by unmapping it from the process's address space.
 *
 * @param vm_page Pointer to the virtual memory page to release.
 * @param units Number of units (pages) to release.
 * @return 0 if the page was successfully released, -1 otherwise.
 */
static int8_t _mm_release_vm_page(void *vm_page, uint32_t units)
{
    return munmap(vm_page, units * SYSTEM_PAGE_SIZE);
}

/**
 * @brief Looks up a struct_record_t object by struct name.
 *
 * This function searches for a struct_record_t object with a matching struct name
 * within the linked list of struct records. It iterates through the list of vm_page_for_struct_records_t
 * objects to find the desired record. If a matching record is found, it is returned.
 *
 * @param struct_name Pointer to the struct name to search for.
 * @return Pointer to the matching struct_record_t object, or NULL if not found.
 */
static struct_record_t *_mm_lookup_struct_record_by_name(char *struct_name)
{
    for (vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL;
         vm_page_record = vm_page_record->next)
    {
        struct_record_t *record = NULL;
        MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record)
        {
            if (strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0)
            {
                return record;
            }
        }
        MM_ITERATE_STRUCT_RECORDS_END;
    }

    return NULL;
}

/**
 * @brief Merges two free memory blocks into a single block.
 *
 * This function merges two free memory blocks into a single block. The function assumes
 * that both input blocks are free (is_free == MM_FREE). It updates the data_block_size,
 * next, and prev fields of the first block to incorporate the second block into a single block.
 *
 * @param first Pointer to the first meta_block_t object.
 * @param second Pointer to the second meta_block_t object.
 */
static void _mm_merge_free_blocks(meta_block_t *first, meta_block_t *second)
{
    assert((first->is_free == MM_FREE) && (second->is_free == MM_FREE));

    first->data_block_size += sizeof(meta_block_t) + second->data_block_size;
    first->next = second->next;
    if (!second->next)
    {
        first->next->prev = first;
    }
}

/**
 * @brief Checks if a data virtual memory page is empty.
 *
 * This function checks if a data virtual memory page is empty by examining its meta_block_info
 * fields, including next, prev, and is_free.
 *
 * @param data_vm_page Pointer to the vm_page_for_data_t object to check.
 * @return MM_FREE if the page is empty, MM_ALLOCATED otherwise.
 */
static vm_bool_t _mm_is_data_vm_page_empty(vm_page_for_data_t *data_vm_page)
{
    if (data_vm_page->meta_block_info.next == NULL && data_vm_page->meta_block_info.prev == NULL &&
        data_vm_page->meta_block_info.is_free == MM_FREE)
    {
        return MM_FREE;
    }

    return MM_ALLOCATED;
}

/**
 * @brief Calculates the maximum amount of virtual memory page memory available.
 *
 * This function calculates the maximum amount of virtual memory page memory available,
 * taking into account the system page size and the offset of the page memory within
 * the vm_page_for_data_t structure.
 *
 * @param units Number of units to calculate memory for.
 * @return Maximum memory available for the specified number of units, in bytes.
 */
static uint32_t _mm_max_vm_page_memory_available(uint32_t units)
{
    return (uint32_t)((SYSTEM_PAGE_SIZE * units) - MM_BLOCK_OFFSETOF(vm_page_for_data_t, page_memory));
}

/**
 * @brief Allocates a virtual memory page for data.
 *
 * This function allocates a virtual memory page for data and initializes its fields.
 *
 * @param record Pointer to the struct_record_t object associated with the data page.
 * @return Pointer to the allocated vm_page_for_data_t object.
 */
static vm_page_for_data_t *mm_allocate_data_vm_page(struct_record_t *record)
{
    vm_page_for_data_t *data_vm_page = (vm_page_for_data_t *)_mm_request_vm_page(1);

    MARK_DATA_VM_PAGE_FREE(data_vm_page);

    data_vm_page->meta_block_info.data_block_size = _mm_max_vm_page_memory_available(1);
    data_vm_page->meta_block_info.offset = MM_BLOCK_OFFSETOF(vm_page_for_data_t, meta_block_info);
    data_vm_page->next = NULL;
    data_vm_page->prev = NULL;
    data_vm_page->record = record;
    glthread_init_node(&data_vm_page->meta_block_info.glue_node);

    if (!record->first_page)
    {
        record->first_page = data_vm_page;
    }
    else
    {
        data_vm_page->next = record->first_page;
        record->first_page->prev = data_vm_page;
        record->first_page = data_vm_page;
    }

    return data_vm_page;
}

/**
 * @brief Deletes and frees a data virtual memory page.
 *
 * This function deletes and frees a data virtual memory page. It removes the page from the associated struct_record_t's
 * page list and releases the virtual memory page using `_mm_release_vm_page`.
 *
 * @param data_vm_page Pointer to the data virtual memory page to delete and free.
 */
static void _mm_delete_and_free_data_vm_page(vm_page_for_data_t *data_vm_page)
{
    struct_record_t *record = data_vm_page->record;

    if (record->first_page == data_vm_page)
    {
        record->first_page = data_vm_page->next;
        if (data_vm_page->next)
        {
            data_vm_page->next->prev = NULL;
        }
        data_vm_page->next = NULL;
        data_vm_page->prev = NULL;
    }
    else
    {
        if (data_vm_page->next)
        {
            data_vm_page->next->prev = data_vm_page->prev;
            data_vm_page->prev->next = data_vm_page->next;
        }
    }

    _mm_release_vm_page((void *)data_vm_page, 1);
}

/**
 * @brief Compares two free memory blocks for sorting.
 *
 * This function compares two free memory blocks for sorting purposes. It compares the data_block_size
 * field of the meta_block_t objects. The function assumes that both input blocks are free (is_free == MM_FREE).
 *
 * @param meta_block_A Pointer to the first meta_block_t object.
 * @param meta_block_B Pointer to the second meta_block_t object.
 * @return -1 if the first block has a larger data_block_size, 1 if the second block has a larger data_block_size,
 *         or 0 if both blocks have the same data_block_size.
 */
static int8_t _mm_free_block_comparison(void *meta_block_A, void *meta_block_B)
{
    meta_block_t *meta_block_A_ptr = (meta_block_t *)meta_block_A;
    meta_block_t *meta_block_B_ptr = (meta_block_t *)meta_block_B;

    assert((meta_block_A_ptr->is_free == MM_FREE) && (meta_block_B_ptr->is_free == MM_FREE));

    if (meta_block_A_ptr->data_block_size > meta_block_B_ptr->data_block_size)
    {
        return -1;
    }
    else if (meta_block_A_ptr->data_block_size < meta_block_B_ptr->data_block_size)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief Adds free data block meta information to a struct_record_t object.
 *
 * This function adds free data block meta information to a struct_record_t object by inserting
 * the meta_block_t object into the free_block_priority_list of the struct_record_t. The insertion
 * is done based on priority using the _mm_free_block_comparison function.
 *
 * @param record Pointer to the struct_record_t object.
 * @param free_meta_block Pointer to the meta_block_t object representing the free data block.
 */
static void _mm_add_free_data_block_meta_info(struct_record_t *record, meta_block_t *free_meta_block)
{
    assert(free_meta_block->is_free == MM_FREE);

    glthread_priority_insert(&record->free_block_priority_list, &free_meta_block->glue_node, _mm_free_block_comparison,
                             MM_BLOCK_OFFSETOF(meta_block_t, glue_node));
}

/**
 * @brief Retrieves the largest free data block from a struct_record_t object.
 *
 * This function retrieves the largest free data block from a struct_record_t object
 * by accessing the head of the free_block_priority_list and converting it to a pointer
 * to the meta_block_t object. The largest free data block is determined based on the
 * priority order defined by the _mm_free_block_comparison function.
 *
 * @param record Pointer to the struct_record_t object.
 * @return Pointer to the largest free meta_block_t object, or NULL if the list is empty.
 */
static meta_block_t *_mm_get_largest_free_data_block(struct_record_t *record)
{
    return (meta_block_t *)(GLTHREAD_BASEOF(record->free_block_priority_list.head,
                                            MM_BLOCK_OFFSETOF(meta_block_t, glue_node)));
}

/**
 * @brief Initializes the memory management system.
 *
 * This function initializes the memory management system by retrieving the system page size
 * using the `sysconf` function and storing it in the `SYSTEM_PAGE_SIZE` global variable.
 * It is typically called at the start of the program to set up the memory management system.
 */
void mm_init(void)
{
    SYSTEM_PAGE_SIZE = sysconf(_SC_PAGESIZE);
}

/**
 * @brief Registers a struct record in the memory management system.
 *
 * This function registers a struct record in the memory management system. The struct record
 * represents a struct with the given name and size. The function checks if the size exceeds
 * the system page size and returns -1 if it does. If the struct name already exists in the
 * record list, it returns -2. Otherwise, it creates a new virtual memory page if needed and
 * inserts the struct record into the appropriate VM page or creates a new VM page for it.
 *
 * @param struct_name The name of the struct to register.
 * @param size The size of the struct.
 * @return 0 if the struct record is registered successfully, -1 if the size exceeds the system page size,
 *         -2 if the struct name already exists in the record list.
 */
int8_t mm_register_struct_record(const char *struct_name, size_t size)
{
    /* we cannot accomodate a struct whose size is greater than the page size */
    if (size > SYSTEM_PAGE_SIZE)
    {
        return -1;
    }
    else
    {
        /* allocating a VM page for the first time */
        if (!vm_page_record_head)
        {
            vm_page_record_head = (vm_page_for_struct_records_t *)_mm_request_vm_page(1);
            vm_page_record_head->next = NULL;
            strncpy(vm_page_record_head->struct_record_list[0].struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE);
            vm_page_record_head->struct_record_list[0].size = size;
            vm_page_record_head->struct_record_list[0].first_page = NULL;
            glthread_init(&vm_page_record_head->struct_record_list[0].free_block_priority_list);
        }
        else
        {
            uint32_t count = 0;
            struct_record_t *record = NULL;
            /* iterating through records in all existing VM pages to insert a record
             * at the end */
            for (vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL;
                 vm_page_record = vm_page_record->next)
            {
                count = 0;
                record = NULL;
                MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record)
                {
                    if (strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0)
                    {
                        /* struct name already exists in the record list */
                        return -2;
                    }
                    else
                    {
                        count++;
                    }
                }
                MM_ITERATE_STRUCT_RECORDS_END;
            }

            if (count == MAX_RECORDS_PER_VM_PAGE)
            {
                /* the previous VM page is full, create a new VM page to add this record
                 */
                vm_page_for_struct_records_t *new_vm_page_record =
                    (vm_page_for_struct_records_t *)_mm_request_vm_page(1);
                new_vm_page_record->next = vm_page_record_head;
                /* the record to be added will be the first record in the new VM page*/
                record = new_vm_page_record->struct_record_list;
                vm_page_record_head = new_vm_page_record;
            }

            strncpy(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE);
            record->size = size;
            record->first_page = NULL;
            glthread_init(&record->free_block_priority_list);
        }
        return 0;
    }
}

/**
 * @brief Prints the registered struct records in the memory management system.
 *
 * This function prints the registered struct records in the memory management system.
 * It iterates through the linked list of vm_page_for_struct_records_t objects and the
 * struct_record_list within each object, printing the struct name and size of each record.
 */
void mm_print_registered_struct_records(void)
{
    for (vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL;
         vm_page_record = vm_page_record->next)
    {
        struct_record_t *record = NULL;
        MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record)
        {
            printf("%s: %ld\n", record->struct_name, record->size);
        }
        MM_ITERATE_STRUCT_RECORDS_END;
    }
}
