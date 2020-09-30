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
    lock();
    peak_size = _peak_size;
    unlock();

    LOG_F(WARNING, "hzip.memory: set peak size: %lu bytes", _peak_size);
}

uint64_t hz_memmgr::get_alloc_count() {
    lock();
    int n = n_allocations;
    unlock();

    return n;
}

uint64_t hz_memmgr::get_alloc_size() {
    lock();
    int size = allocation_size;
    unlock();

    return size;
}

void hz_memmgr::set_parent(hz_memmgr *p) {
    parent = p;
}

uint64_t hz_memmgr::get_peak_size() {
    lock();
    int size = peak_size;
    unlock();

    return size;
}

void hz_memmgr::update(uint64_t alloc_size, uint64_t alloc_count) {
    lock();

    allocation_size = alloc_size;
    n_allocations = alloc_count;

    unlock();
}

void hz_memmgr::lock() {
    sem_wait(&mutex);
}

void hz_memmgr::unlock() {
    sem_post(&mutex);
}

void hz_memmgr::wipe() {

}
