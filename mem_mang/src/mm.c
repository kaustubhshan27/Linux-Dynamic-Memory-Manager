#include <stdio.h>
#include <memory.h>
#include <stdint.h>
#include <unistd.h>     /* for getpagesize */
#include <sys/mman.h>   /* for mmap() */

static size_t SYSTEM_PAGE_SIZE = 0; /* size of VM page on system */

/* To initialize the memory manager */
void mm_init() {
    SYSTEM_PAGE_SIZE = getpagesize();
}

/* To request VM page from the kernel */
static void *request_vm_page(uint32_t units) {
    char *vm_page = mmap(0, units*SYSTEM_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE, 0, 0);
    if(vm_page == MAP_FAILED){
        printf("Error : VM page allocation failed\n");
        return NULL;
    }
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);
    return (void *)vm_page;
}

/* To release VM page to the kernel */
static void release_vm_page(void *vm_page, uint32_t units) {
    if(munmap(vm_page, units * SYSTEM_PAGE_SIZE)){
        printf("Error : Could not release VM page to kernel\n");
    }
}

int main() {
    mm_init();
    printf("VM page size = %lu\n", SYSTEM_PAGE_SIZE);
    void *addr1 = request_vm_page(1);
    void *addr2 = request_vm_page(1);
    //printf("addr1 = %p --- addr2 = %p\n", addr1, addr2);
    return 0;
}