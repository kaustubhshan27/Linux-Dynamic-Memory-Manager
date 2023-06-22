#include "mm.h"

/* size of a VM page on this system */
static size_t SYSTEM_PAGE_SIZE = 0;

/* pointer to the current head of the VM page lists containing struct records */
static vm_page_for_struct_records_t *vm_page_record_head = NULL;

/* to request VM page from the kernel */
static void *_mm_request_vm_page(uint32_t units) {
    /* the virtual mapping should be done in the heap */
    uint8_t *vm_page = mmap(sbrk(0), units * SYSTEM_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    if(vm_page == MAP_FAILED){
        return NULL;
    }
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);

    return (void *)vm_page;
}

/* to release VM page to the kernel */
static int8_t _mm_release_vm_page(void *vm_page, uint32_t units) {
    return munmap(vm_page, units * SYSTEM_PAGE_SIZE);
}

/* to lookup a particular struct name in all VM page records */
static struct_record_t *_mm_lookup_struct_record_by_name(char *struct_name) {
    for(vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL; vm_page_record = vm_page_record->next) {
        struct_record_t *record = NULL;
        MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
            if(strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0) {
                return record;
            }
        } MM_ITERATE_STRUCT_RECORDS_END;
    }

    return NULL;
}

static void _mm_merge_free_blocks(meta_block_t *first, meta_block_t *second) {
    assert(first->is_free == MM_FREE && second->is_free == MM_FREE);

    first->data_block_size += sizeof(meta_block_t) + second->data_block_size;
    first->next = second->next;
    if(!second->next) {
        first->next->prev = first;
    }
}

/* checks if a data VM page is totally free */
static vm_bool_t _mm_is_data_vm_page_empty(vm_page_for_data_t *data_vm_page) {
    if(data_vm_page->meta_block_info.next == NULL && data_vm_page->meta_block_info.prev == NULL && data_vm_page->meta_block_info.is_free == MM_FREE) {
        return MM_FREE;
    }

    return MM_ALLOCATED;
}

/* returns the size of the free memory available in contiguous data VM pages */
static uint32_t _mm_max_vm_page_memory_available(uint32_t units) {
    return (uint32_t)((SYSTEM_PAGE_SIZE * units) - MM_BLOCK_OFFSETOF(vm_page_for_data_t, page_memory));
}

/* allocates a data VM page and accordingly adjust the list links */
static vm_page_for_data_t *mm_allocate_data_vm_page(struct_record_t *record) {
    vm_page_for_data_t *data_vm_page = (vm_page_for_data_t *)_mm_request_vm_page(1);

    MARK_DATA_VM_PAGE_FREE(data_vm_page);

    data_vm_page->meta_block_info.data_block_size = _mm_max_vm_page_memory_available(1);
    data_vm_page->meta_block_info.offset = MM_BLOCK_OFFSETOF(vm_page_for_data_t, meta_block_info);
    data_vm_page->next = NULL;
    data_vm_page->prev = NULL;
    data_vm_page->record = record;

    if(!record->first_page) {
        record->first_page = data_vm_page;
    } else {
        data_vm_page->next = record->first_page;
        record->first_page->prev = data_vm_page;
        record->first_page = data_vm_page;
    }

    return data_vm_page;;
}

/* deletes and returns a data VM page */
static void mm_delete_and_free_data_vm_page(vm_page_for_data_t *data_vm_page) {
    struct_record_t *record = data_vm_page->record;

    if(record->first_page == data_vm_page) {
        record->first_page = data_vm_page->next;
        if(data_vm_page->next) {
            data_vm_page->next->prev = NULL;
        }
        data_vm_page->next = NULL;
        data_vm_page->prev = NULL;
    } else {
        if(data_vm_page->next) {
            data_vm_page->next->prev = data_vm_page->prev;
            data_vm_page->prev->next = data_vm_page->next;
        }
    }

    _mm_release_vm_page((void *)data_vm_page, 1);
}

void mm_init(void) {
    SYSTEM_PAGE_SIZE = sysconf(_SC_PAGESIZE);
}

int8_t mm_register_struct_record(const char *struct_name, size_t size) {
    /* we cannot accomodate a struct whose size is greater than the page size */
    if(size > SYSTEM_PAGE_SIZE) {
        return -1;
    } else {
        /* allocating a VM page for the first time */
        if(!vm_page_record_head) {
            vm_page_record_head = (vm_page_for_struct_records_t *)_mm_request_vm_page(1);
            vm_page_record_head->next = NULL;
            strncpy(vm_page_record_head->struct_record_list[0].struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE);
            vm_page_record_head->struct_record_list[0].size = size;
        } else {
            uint32_t count = 0;
            struct_record_t *record = NULL;
            /* iterating through records in all existing VM pages to insert a record at the end */
            for(vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL; vm_page_record = vm_page_record->next) {
                count = 0;
                record = NULL;
                MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
                    if(strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0) {
                        /* struct name already exists in the record list */
                        return -2;
                    } else {
                        count++;
                    }
                } MM_ITERATE_STRUCT_RECORDS_END;
            }

            if(count == MAX_RECORDS_PER_VM_PAGE) {
                /* the previous VM page is full, create a new VM page to add this record */
                vm_page_for_struct_records_t *new_vm_page_record = (vm_page_for_struct_records_t *)_mm_request_vm_page(1);
                new_vm_page_record->next = vm_page_record_head;
                /* the record to be added will be the first record in the new VM page*/
                record = new_vm_page_record->struct_record_list;
                vm_page_record_head = new_vm_page_record;
            }

            strncpy(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE);
            record->size = size;
        }
        return 0;
    }
}

void mm_print_registered_struct_records(void) {
    for(vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL; vm_page_record = vm_page_record->next) {
        struct_record_t *record = NULL;
        MM_ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
            printf("%s: %ld\n", record->struct_name, record->size);
        } MM_ITERATE_STRUCT_RECORDS_END;
    }
}
