#ifndef HYBRIDZIP_MEMMAP_H
#define HYBRIDZIP_MEMMAP_H

#include <cstdint>
#include <memory>

struct hz_map_elem {
    void *ptr;
    uint64_t alloc_size;
    hz_map_elem *next;
};

struct hz_memmap {
private:
    uint64_t hash(void *ptr);


public:
    uint64_t max_size;
    hz_map_elem **mapptr;

    hz_memmap(uint64_t size);


    void add(hz_map_elem *elem);


    hz_map_elem *get(void *ptr);


    void remove(void *ptr);
};

#endif
