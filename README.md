# Linux-Dynamic-Memory-Manager
The project implements its own malloc() and free() calls to allocate and free memory dynamically. 

The memory manager uses a linked list of free blocks to track available memory. When xcalloc() is called, the memory manager searches the free list for a block that is large enough to satisfy the request. If a suitable block is found, the block is removed from the free list and returned to the caller. If no suitable block is found, a new block is allocated from the operating system. When xfree() is called, the memory manager adds the freed block to the free list.

The memory manager includes features that improve performance and memory utilization, such as merging blocks together to avoid fragmentation.

A demo of memory allocation and deallocation:


![lhmm](https://github.com/kaustubhshan27/linux-heap-memory-manager/assets/32894621/1047f340-1cb4-4e9a-88f1-63579e203f39)
