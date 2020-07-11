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

    ~hz_memmap() {
        free(mapptr);
    }

    void add(hz_map_elem *elem);


    hz_map_elem *get(void *ptr);

    template <typename Type>
    void remove_by_type(Type ptr) {
        uint64_t ptr_hash = hash(ptr);
        auto curr = mapptr[ptr_hash];
        auto prev = curr;


        while (curr != nullptr && curr->ptr != ptr) {
            prev = curr;
            curr = curr->next;
        }

        if (prev != curr) {
            prev->next = curr->next;
        } else {
            mapptr[ptr_hash] = curr->next;
        }

        delete[] ptr;
        delete curr;
    }
};

#endif
