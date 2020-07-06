#ifndef HYBRIDZIP_MEMMGR_H
#define HYBRIDZIP_MEMMGR_H

#include <cstdint>
#include "memmap.h"

struct hz_memmgr {
    uint64_t allocation_size;
    uint64_t n_allocations;
    hz_memmap *memmap;

    hz_memmgr();

    template<typename T>
    T *hz_malloc(int n_elems) {
        auto elem = new hz_map_elem;
        elem->ptr = new T[n_elems];
        elem->alloc_size = n_elems * sizeof(T);
        memmap->add(elem);

        allocation_size += elem->alloc_size;
        n_allocations += 1;

        return (T*) elem->ptr;
    }

    void hz_free(void *ptr);
};

#endif
