#ifndef HYBRIDZIP_MEMMAP_H
#define HYBRIDZIP_MEMMAP_H

#include <cstdint>
#include <memory>

struct hz_map_elem {
    void *ptr;
    uint64_t alloc_size;
    hz_map_elem *next;
    hz_map_elem *next_iter;
    hz_map_elem *prev_iter;
};

struct hz_memmap {
private:
    uint64_t hash(void *ptr);

public:
    uint64_t max_size;
    hz_map_elem **mapptr;
    hz_map_elem *iterptr = nullptr;
    hz_map_elem *head = nullptr;

    hz_memmap(uint64_t size);

    ~hz_memmap() {
        delete mapptr;
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

        // Remove curr from the iteration linked-list
        if (curr->prev_iter == nullptr) {
            head = curr->next_iter;
            if (head == nullptr) {
                iterptr = nullptr;
            }
        } else if (curr->next_iter == nullptr) {
            iterptr = iterptr->prev_iter;
        } else {
            curr->prev_iter->next_iter = curr->next_iter;
            curr->next_iter->prev_iter = curr->prev_iter;
        }

        delete curr;
    }
};

#endif
