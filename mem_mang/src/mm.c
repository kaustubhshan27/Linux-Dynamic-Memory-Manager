#include "../inc/mm.h"

/* size of VM page on system */
static size_t SYSTEM_PAGE_SIZE = 0;

/* to initialize the memory manager */
void mm_init(void) {
    SYSTEM_PAGE_SIZE = getpagesize();
}

/* to request VM page from the kernel */
static void *request_vm_page(uint32_t units) {
    /* the virtual mapping should be done in the heap */
    uint8_t *vm_page = mmap(sbrk(0), units*SYSTEM_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    if(vm_page == MAP_FAILED){
        printf("Error: VM page allocation failed\n");
        return NULL;
    }
    memset(vm_page, 0, units*SYSTEM_PAGE_SIZE);

    return (void *)vm_page;
}

/* to release VM page to the kernel */
static void release_vm_page(void *vm_page, uint32_t units) {
    if(munmap(vm_page, units*SYSTEM_PAGE_SIZE)){
        printf("Error: Could not release VM page to kernel\n");
    }
}
