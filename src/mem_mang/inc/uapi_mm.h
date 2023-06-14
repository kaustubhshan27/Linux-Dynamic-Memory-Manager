#ifndef _UAPI_MEM_MANG_
#define _UAPI_MEM_MANG_

#include <stdint.h>
#include <stddef.h>

/* to initialize the memory manager */
void mm_init(void);

/* to add a struct record <struct name, struct size> to a list in a VM page */
int8_t mm_register_struct_record(const char *struct_name, size_t size);

/* prints the name of the structs currently registered by the memory manager */
void mm_print_registered_struct_records(void);

#define MM_REG_STRUCT(struct_name) \
    mm_register_struct_record(#struct_name, sizeof(struct_name))

#endif /* UAPI_MEM_MANG_ */