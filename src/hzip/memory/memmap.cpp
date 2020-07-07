#include <iostream>
#include "memmap.h"

hz_memmap::hz_memmap(uint64_t size) {
    mapptr =  (hz_map_elem **)malloc(sizeof(hz_map_elem*) * size);
    for (int i = 0; i < size; i++) {
        mapptr[i] = nullptr;
    }
    max_size = size;
}

void hz_memmap::add(hz_map_elem *elem) {
    uint64_t ptr_hash = hash(elem->ptr);
    elem->next = mapptr[ptr_hash];
    mapptr[ptr_hash] = elem;
}

hz_map_elem *hz_memmap::get(void *ptr) {
    uint64_t ptr_hash = hash(ptr);
    auto curr = mapptr[ptr_hash];

    while (curr != nullptr && curr->ptr != ptr) {
        curr = curr->next;
    }

    return curr;
}

void hz_memmap::remove(void *ptr) {
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

    free(curr->ptr);
    free(curr);
}

uint64_t hz_memmap::hash(void *ptr) {
    auto value = (uint64_t) ptr;
    value = value + value ^ (value >> 2);
    return value % max_size;
}

