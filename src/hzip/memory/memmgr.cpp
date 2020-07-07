#include "memmgr.h"

hz_memmgr::hz_memmgr() {
    memmap = new hz_memmap(0xFFFF);
    n_allocations = 0;
    allocation_size = 0;
}

void hz_memmgr::hz_free(void *ptr) {
    n_allocations -= 1;
    allocation_size -= memmap->get(ptr)->alloc_size;
    memmap->remove(ptr);
}

