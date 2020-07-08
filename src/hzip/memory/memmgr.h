#ifndef HYBRIDZIP_MEMMGR_H
#define HYBRIDZIP_MEMMGR_H

#include <cstdint>
#include "memmap.h"

#define HZ_MALLOC(type, n_elems) this->__hz_memmgr_obj->hz_malloc<type>(n_elems)
#define HZ_FREE(ptr) this->__hz_memmgr_obj->hz_free(ptr)
#define HZ_MEM_INIT(child) child.attach_memmgr(this->__hz_memmgr_obj)

struct hz_memmgr {
    uint64_t allocation_size;
    uint64_t n_allocations;
    hz_memmap *memmap;

    hz_memmgr();

    ~hz_memmgr() {
        memmap->~hz_memmap();
        delete memmap;
    }

    template<typename T>
    T *hz_malloc(int n_elems) {
        auto elem = (hz_map_elem*)malloc(sizeof(hz_map_elem));

        elem->ptr = malloc(sizeof(T) * n_elems);
        elem->alloc_size = n_elems * sizeof(T);
        elem->next = nullptr;

        memmap->add(elem);

        allocation_size += elem->alloc_size;
        n_allocations += 1;

        return (T*) elem->ptr;
    }

    void hz_free(void *ptr);


};

#endif
