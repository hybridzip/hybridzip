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
    hz_memmgr *parent;
    sem_t mutex;

    void lock();

    void unlock();

public:
    hz_memmgr();

    ~hz_memmgr() {
        sem_wait(&mutex);

        delete memmap;

        sem_post(&mutex);
    }

    template<typename Type>
    Type *hz_malloc(int n_elems) {
        lock();

        uint64_t curr_alloc_size = sizeof(Type) * n_elems;

        if (peak_size != 0 && allocation_size + curr_alloc_size > peak_size) {
            unlock();
            throw MemoryErrors::PeakLimitReachedException();
        }

        if (parent != nullptr &&
            parent->peak_size != 0 &&
            parent->get_alloc_size() + curr_alloc_size > parent->get_peak_size()) {
            unlock();
            throw MemoryErrors::PeakLimitReachedException();
        }

        auto elem = new hz_map_elem;

        elem->ptr = new Type[n_elems];
        elem->alloc_size = n_elems * sizeof(Type);
        elem->next = nullptr;

        memmap->add(elem);

        allocation_size += elem->alloc_size;
        n_allocations += 1;

        if (parent != nullptr) {
            parent->update(parent->get_alloc_size() + elem->alloc_size,
                    parent->get_alloc_count() + 1);
        }

        unlock();

        return (Type *) elem->ptr;
    }

    template<typename Type>
    void hz_free(Type ptr) {
        if (ptr == nullptr) {
            return;
        }

        lock();

        n_allocations -= 1;

        auto *elem =  memmap->get((void *) ptr);
        if (elem == nullptr) {
            unlock();
            return;
        }

        allocation_size -= elem->alloc_size;
        memmap->remove_by_type<Type>(ptr);

        unlock();
    }

    void set_peak(uint64_t _peak_size);

    void set_parent(hz_memmgr *p);

    uint64_t get_alloc_count();

    uint64_t get_alloc_size();

    uint64_t get_peak_size();

    void update(uint64_t alloc_size, uint64_t alloc_count);

    void wipe();
};

#endif
