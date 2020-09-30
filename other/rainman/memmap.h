#ifndef RAINMAN_MEMMAP_H
#define RAINMAN_MEMMAP_H

#include <cstdint>
#include <memory>

namespace rainman {
    struct map_elem {
        void *ptr = nullptr;
        uint64_t alloc_size = 0;
        const char *type_name = nullptr;
        map_elem *next = nullptr;
        map_elem *next_iter = nullptr;
        map_elem *prev_iter = nullptr;
    };

    struct memmap {
    private:

        uint64_t hash(void *ptr);

    public:
        uint64_t max_size;
        map_elem **mapptr;
        map_elem *iterptr = nullptr;
        map_elem *head = nullptr;

        memmap(uint64_t size);

        ~memmap() {
            delete mapptr;
        }

        void add(map_elem *elem);


        map_elem *get(void *ptr);

        template<typename Type>
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
                } else {
                    head->prev_iter = nullptr;
                }
            } else if (curr->next_iter == nullptr) {
                iterptr = curr->prev_iter;
                if (iterptr == nullptr) {
                    head = nullptr;
                } else {
                    iterptr->next_iter = nullptr;
                }
            } else {
                curr->prev_iter->next_iter = curr->next_iter;
                curr->next_iter->prev_iter = curr->prev_iter;
            }

            delete curr;
        }
    };
}


#endif
