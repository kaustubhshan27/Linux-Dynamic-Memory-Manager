#include "../inc/mm.h"

/* size of a VM page on this system */
static size_t SYSTEM_PAGE_SIZE = 0;

/* pointer to the current head of the VM page lists containing struct records */
static vm_page_for_struct_records_t *vm_page_record_head = NULL;

/* to request VM page from the kernel */
static void *_request_vm_page(uint32_t units) {
    /* the virtual mapping should be done in the heap */
    uint8_t *vm_page = mmap(sbrk(0), units * SYSTEM_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    if(vm_page == MAP_FAILED){
        return NULL;
    }
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);

    return (void *)vm_page;
}

/* to release VM page to the kernel */
static int8_t _release_vm_page(void *vm_page, uint32_t units) {
    return munmap(vm_page, units * SYSTEM_PAGE_SIZE);
}

/* to lookup a particular struct name in all VM page records */
static struct_record_t *lookup_struct_record_by_name(char *struct_name) {
    for(vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL; vm_page_record = vm_page_record->next) {
        struct_record_t *record = NULL;
        ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
            if(strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0) {
                return record;
            }
        } ITERATE_STRUCT_RECORDS_END
    }

    return NULL;
}

void mm_init(void) {
    SYSTEM_PAGE_SIZE = sysconf(_SC_PAGESIZE);
}

int8_t mm_register_struct_record(char *struct_name, size_t size) {
    /* we cannot accomodate a struct whose size is greater than the page size */
    if(size > SYSTEM_PAGE_SIZE) {
        return -1;
    } else {
        /* allocating a VM page for the first time */
        if(!vm_page_record_head) {
            vm_page_record_head = (vm_page_for_struct_records_t *)_request_vm_page(1);
            vm_page_record_head->next = NULL;
            strncpy(vm_page_record_head->struct_record_list->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE);
            vm_page_record_head->struct_record_list->size = size;
        } else {
            uint32_t count = 0;
            struct_record_t *record = NULL;
            /* iterating through records in all existing VM pages to insert a record at the end */
            for(vm_page_for_struct_records_t *vm_page_record = vm_page_record_head; vm_page_record != NULL; vm_page_record = vm_page_record->next) {
                count = 0;
                record = NULL;
                ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
                    if(strncmp(record->struct_name, struct_name, MM_MAX_STRUCT_NAME_SIZE) == 0) {
                        /* struct name already exists in the record list */
                        return -2;
                    } else {
                        count++;
                    }
                } ITERATE_STRUCT_RECORDS_END
            }

            if(count == MAX_RECORDS_PER_VM_PAGE) {
                /* the previous VM page is full, create a new VM page to add this record */
                vm_page_for_struct_records_t *new_vm_page_record = (vm_page_for_struct_records_t *)_request_vm_page(1);
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
        ITERATE_STRUCT_RECORDS_BEGIN(vm_page_record->struct_record_list, record) {
            printf("%s: %d\n", record->struct_name, record->size);
        } ITERATE_STRUCT_RECORDS_END
    }
}
