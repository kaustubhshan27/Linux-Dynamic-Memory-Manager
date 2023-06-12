#ifndef _MEM_MANG_
#define _MEM_MANG_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
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

/* to initialize the memory manager */
void mm_init(void);
/* to add a struct record <struct name, struct size> to a list in a VM page */
int8_t mm_register_struct_record(char *struct_name, size_t size);

#endif /* _MEM_MANG_ */
