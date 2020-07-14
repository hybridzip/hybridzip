#include "memmgr.h"
#include <loguru/loguru.hpp>

hz_memmgr::hz_memmgr() {
    sem_init(&mutex, 0, 1);
    memmap = new hz_memmap(0xFFFF);
    n_allocations = 0;
    allocation_size = 0;
    peak_size = 0;
    parent = nullptr;
}

void hz_memmgr::set_peak(uint64_t _peak_size) {
    sem_wait(&mutex);
    peak_size = _peak_size;
    sem_post(&mutex);

    LOG_F(WARNING, "hzip.memory: set peak size: %lu bytes", _peak_size);
}

uint64_t hz_memmgr::get_alloc_count() {
    sem_wait(&mutex);
    int n = n_allocations;
    sem_post(&mutex);

    return n;
}

uint64_t hz_memmgr::get_alloc_size() {
    sem_wait(&mutex);
    int size = allocation_size;
    sem_post(&mutex);

    return size;
}

void hz_memmgr::set_parent(hz_memmgr *p) {
    parent = p;
}

uint64_t hz_memmgr::get_peak_size() {
    sem_wait(&mutex);
    int size = peak_size;
    sem_post(&mutex);

    return size;
}

void hz_memmgr::update(uint64_t alloc_size, uint64_t alloc_count) {
    sem_wait(&mutex);

    allocation_size = alloc_size;
    n_allocations = alloc_count;

    sem_post(&mutex);
}
