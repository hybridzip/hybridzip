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

        auto elem = new hz_map_elem;

        elem->ptr = new T[n_elems];
        elem->alloc_size = n_elems * sizeof(T);
        elem->next = nullptr;

        memmap->add(elem);

        allocation_size += elem->alloc_size;
        n_allocations += 1;

        sem_post(&mutex);

        return (T *) elem->ptr;
    }

    template <typename Type>
    void hz_free(Type ptr) {
        if (ptr == nullptr) {
            return;
        }

        sem_wait(&mutex);

        n_allocations -= 1;
        allocation_size -= memmap->get((void*) ptr)->alloc_size;
        memmap->remove_by_type<Type>(ptr);

        sem_post(&mutex);
    }

    void set_peak(uint64_t _peak_size);

    uint64_t get_alloc_count();

    uint64_t get_alloc_size();
};

#endif
