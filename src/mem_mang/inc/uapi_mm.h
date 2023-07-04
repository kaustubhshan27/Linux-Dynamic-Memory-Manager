#ifndef _UAPI_MEM_MANG_
#define _UAPI_MEM_MANG_

#include <stddef.h>
#include <stdint.h>

void mm_init(void);
int8_t mm_register_struct_record(const char *struct_name, size_t size);
void mm_print_registered_struct_records(void);
void *xcalloc(const char *struct_name, uint32_t units);
void mm_print_mem_usage(const char *struct_name);
void mm_print_block_usage(void);

#define MM_REG_STRUCT(struct_name) mm_register_struct_record(#struct_name, sizeof(struct_name))

#endif /* UAPI_MEM_MANG_ */