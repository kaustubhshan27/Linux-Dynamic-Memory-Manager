#ifndef _MEM_MANG_
#define _MEM_MANG_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h> /* for mmap() and munmap() */
#include <unistd.h>   /* for sysconf(_SC_PAGESIZE) */

typedef enum
{
    MM_FREE,
    MM_ALLOCATED
} vm_bool_t;

typedef struct meta_block
{
    vm_bool_t is_free;
    uint32_t data_block_size;
    struct meta_block *prev;
    struct meta_block *next;
    uint32_t offset;
} meta_block_t;

#define MM_BLOCK_OFFSETOF(struct_type, field_name) (size_t)(&((struct_type *)NULL)->field_name)

#define MM_GET_PAGE_FROM_META_BLOCK(meta_block_ptr)                                                                    \
    (void *)((uint8_t *)meta_block_ptr - ((meta_block_t *)meta_block_ptr)->offset)

#define MM_NEXT_META_BLOCK(meta_block_ptr) (meta_block_t *)(((meta_block_t *)meta_block_ptr)->next)

#define MM_NEXT_META_BLOCK_BY_SIZE(meta_block_ptr)                                                                     \
    (meta_block_t *)((uint8_t *)meta_block_ptr + sizeof(meta_block_t) + ((meta_block_t *)meta_block_ptr)->size)

#define MM_PREV_META_BLOCK(meta_block_ptr) (meta_block_t *)(((meta_block_t *)meta_block_ptr)->prev)

#define MM_BIND_BLOCKS_AFTER_SPLITTING(allocated_meta_block, free_meta_block)                                          \
    ((meta_block_t *)free_meta_block)->next =                                                                          \
        ((meta_block_t *)allocated_meta_block)->next((meta_block_t *)free_meta_block)->prev =                          \
            (meta_block_t *)allocated_meta_block if (((meta_block_t *)free_meta_block)->next)(                         \
                (meta_block_t *)allocated_meta_block)                                                                  \
                ->next->prev = (meta_block_t *)free_meta_block((meta_block_t *)allocated_meta_block)->next =           \
                (meta_block_t *)free_meta_block

typedef struct vm_page_for_data
{
    struct vm_page_for_data *prev;
    struct vm_page_for_data *next;
    struct struct_record *record;
    meta_block_t meta_block_info;
    uint8_t page_memory[];
} vm_page_for_data_t;

#define MARK_DATA_VM_PAGE_FREE(vm_page_for_data_ptr)                                                                   \
    ((vm_page_for_data_t *)vm_page_for_data_ptr)->meta_block_info.prev = NULL;                                         \
    ((vm_page_for_data_t *)vm_page_for_data_ptr)->meta_block_info.next = NULL;                                         \
    ((vm_page_for_data_t *)vm_page_for_data_ptr)->meta_block_info.is_free = MM_FREE;

#define MM_ITERATE_DATA_VM_PAGES_BEGIN(struct_record_ptr, data_vm_page_ptr)                                            \
    {                                                                                                                  \
        for (data_vm_page_ptr = ((vm_page_for_data_t *)struct_record_ptr)->first_vm_data_page;                         \
             data_vm_page_ptr != NULL; data_vm_page_ptr = ((vm_page_for_data_t *)data_vm_page_ptr)->next)              \
        {

#define MM_ITERATE_DATA_VM_PAGES_END                                                                                   \
    }                                                                                                                  \
    }

#define MM_ITERATE_ALL_BLOCKS_OF_SINGLE_DATA_VM_PAGE_BEGIN(data_vm_page_ptr, meta_block_ptr)                           \
    {                                                                                                                  \
        for (meta_block_ptr = (meta_block_t *)(&(((vm_page_for_data_t *)data_vm_page_ptr)->meta_block_info));          \
             meta_block_ptr != NULL; meta_block_ptr = ((meta_block_t *)meta_block_ptr)->next)                          \
        {

#define MM_ITERATE_ALL_BLOCKS_OF_SINGLE_DATA_VM_PAGE_END                                                               \
    }                                                                                                                  \
    }

#define MAX_RECORDS_PER_VM_PAGE ((SYSTEM_PAGE_SIZE - sizeof(vm_page_for_struct_records_t *)) / sizeof(struct_record_t))

#define MM_MAX_STRUCT_NAME_SIZE 32
typedef struct struct_record
{
    char struct_name[MM_MAX_STRUCT_NAME_SIZE];
    size_t size;
    struct vm_page_for_data *first_page;
} struct_record_t;

typedef struct vm_page_for_struct_records
{
    struct vm_page_for_struct_records *next;
    /* to point to the list of structs in the VM page - struct hack VLA used */
    struct_record_t struct_record_list[];
} vm_page_for_struct_records_t;

#define MM_ITERATE_STRUCT_RECORDS_BEGIN(record_list_ptr, record)                                                       \
    {                                                                                                                  \
        uint32_t limit = 0;                                                                                            \
        for (record = (struct_record_t *)record_list_ptr; limit < MAX_RECORDS_PER_VM_PAGE && record->size;             \
             record++, limit++)                                                                                        \
        {

#define MM_ITERATE_STRUCT_RECORDS_END                                                                                  \
    }                                                                                                                  \
    }

#endif /* _MEM_MANG_ */
