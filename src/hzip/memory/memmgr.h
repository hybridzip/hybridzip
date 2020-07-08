#ifndef HYBRIDZIP_MEMMGR_H
#define HYBRIDZIP_MEMMGR_H

#include <cstdint>
#include <semaphore.h>
#include <hzip/errors/memory.h>
#include "memmap.h"


class hz_memmgr {
private:
    uint64_t allocation_size;
    uint64_t n_allocations;
    uint64_t peak_size;
    hz_memmap *memmap;
    sem_t mutex;

public:
    hz_memmgr();

    ~hz_memmgr() {
        sem_wait(&mutex);

        memmap->~hz_memmap();
        delete memmap;

        sem_post(&mutex);
    }

    template<typename T>
    T *hz_malloc(int n_elems) {
        sem_wait(&mutex);

        uint64_t curr_alloc_size = sizeof(T) * n_elems;
        if (peak_size != 0 && allocation_size + curr_alloc_size > peak_size) {
            sem_post(&mutex);
            throw MemoryErrors::PeakLimitReachedException();
        }

        auto elem = (hz_map_elem *) malloc(sizeof(hz_map_elem));

        elem->ptr = malloc(sizeof(T) * n_elems);
        elem->alloc_size = n_elems * sizeof(T);
        elem->next = nullptr;

        memmap->add(elem);

        allocation_size += elem->alloc_size;
        n_allocations += 1;

        sem_post(&mutex);

        return (T *) elem->ptr;
    }

    void hz_free(void *ptr);

    void set_peak(uint64_t _peak_size);

    uint64_t get_alloc_count();

    uint64_t get_alloc_size();
};

#endif
