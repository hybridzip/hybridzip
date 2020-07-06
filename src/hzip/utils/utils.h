#ifndef HYBRIDZIP_UTILS_H
#define HYBRIDZIP_UTILS_H

#include <functional>
#include <cstdint>
#include <stack>
#include "common.h"

// A light-weight efficient stack that maintains a stack of buffers to minimize memory allocations.
template <typename T>
struct hz_stack {
    struct hz_buffer {
        T *data;
        hz_buffer *next;
        hz_buffer *prev;

        hz_buffer(uint64_t size) {
            data = new T[size];
            next = nullptr;
            prev = nullptr;
        }

        void destroy() {
            free(data);
        }
    };

    hz_buffer *buffer;
    const uint64_t bufsize = 0xFFF;
    uint64_t index;

    hz_stack() {
        buffer = new hz_buffer(bufsize);
        index = 0;
    }

     void push(T x) {
        if (buffer == nullptr) {
            buffer = new hz_buffer(bufsize);
        }

         if (index < bufsize) {
             buffer->data[index++] = x;
         } else {
             buffer->next = new hz_buffer(bufsize);
             buffer->next->prev = buffer;
             buffer = buffer->next;
             index = 0;
             buffer->data[index++] = x;
         }
     }

     void pop() {
        if (index > 1) {
            index--;
        } else {
            index = bufsize;
            buffer = buffer->prev;

            if (buffer != nullptr) {
                buffer->next->destroy();
                free(buffer->next);
            }
        }
    }

    T top() {
        return buffer->data[index-1];
    }

    uint64_t size() {
        uint64_t count = index;
        auto tmp = buffer->prev;

        while (tmp != nullptr) {
            tmp = tmp->prev;
            count += bufsize;
        }

        return count;
    }

    bool empty() {
        return buffer == nullptr || (buffer->prev == nullptr && index == 0);
    }
};

HZ_INLINE uint64_t u64log2(uint64_t n) {
    if (n == 0) return 0;
    uint64_t count = 0;
    while (n > 0) {
        n >>= 1;
        count++;
    }
    return --count;
}

HZ_INLINE bin_t unarypx_bin(uint64_t n) {
    if (n == 0) return bin_t{.obj=0, .n=1};
    auto lg2 = u64log2(n);
    auto lg2_copy = lg2;
    uint64_t num = 1;

    while (lg2_copy) {
        num <<= 1;
        num++;
        lg2_copy--;
    }

    num <<= 2; // add 0 at the end for unary.
    num <<= lg2; // lshift by logn bits
    num += n; // add n as suffix
    return bin_t{.obj=num, .n=(HZ_UINT) (1 + ((1 + lg2) << 1))};
}

template <typename T>
HZ_INLINE bin_t unaryinv_bin(T readfunc) {
    uint64_t count = 0;

    while (readfunc(1) != 0) {
        count++;
    }

    uint64_t obj = readfunc(count);
    return bin_t{.obj=obj, .n=(HZ_UINT) count};
}

#endif
