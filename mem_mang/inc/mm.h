#ifndef _MEM_MANG_
#define _MEM_MANG_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     /* for sysconf(_SC_PAGESIZE) */
#include <sys/mman.h>   /* for mmap() and munmap() */

#define MAX_RECORDS_PER_VM_PAGE ((SYSTEM_PAGE_SIZE - sizeof(vm_page_for_struct_records_t *)) / sizeof(struct_record_t))

#define MM_MAX_STRUCT_NAME_SIZE 32

typedef struct struct_record {
    char struct_name[MM_MAX_STRUCT_NAME_SIZE];
    size_t size;
} struct_record_t;

typedef struct vm_page_for_struct_records {
    struct vm_page_for_struct_records *next;
    struct_record_t *struct_record_list;
} vm_page_for_struct_records_t;

#define ITERATE_STRUCT_RECORDS_BEGIN(record_list_ptr, record)                                   \
{                                                                                               \
    uint32_t limit = 0;                                                                         \
    for(record = (struct_record_t *)record_list_ptr;                                            \ 
        limit < MAX_RECORDS_PER_VM_PAGE && record->size;                                        \
        record++, limit++) {
         
#define ITERATE_STRUCT_RECORDS_END    }}

typedef enum {
    FREE, 
    ALLOCATED
} data_block_state_t;

typedef struct meta_block {
    data_block_state_t is_free;
    uint32_t data_block_size;
    struct meta_block *prev;
    struct meta_block *next;
    uint32_t offset;
} meta_block_t;

#define BLOCK_OFFSETOF(struct_type, field_name)     \
    (size_t)(&((struct_type *)NULL)->field_name)

#define GET_PAGE_FROM_META_BLOCK(meta_block_ptr)    \
    (void *)((uint8_t *)meta_block_ptr - ((meta_block_t *)meta_block_ptr)->offset)

#define NEXT_META_BLOCK(meta_block_ptr)             \
    (meta_block_t *)(((meta_block_t *)meta_block_ptr)->next)

#define NEXT_META_BLOCK_BY_SIZE(meta_block_ptr)     \
    (meta_block_t *)((uint8_t *)meta_block_ptr + sizeof(meta_block_t) + ((meta_block_t *)meta_block_ptr)->size)

#define PREV_META_BLOCK(meta_block_ptr)             \
    (meta_block_t *)(((meta_block_t *)meta_block_ptr)->prev)

#endif /* _MEM_MANG_ */
