#ifndef HYBRIDZIP_UTILS_H
#define HYBRIDZIP_UTILS_H

#include <functional>
#include <cstdint>
#include <stack>
#include "common.h"

// A light-weight efficient stack that maintains a stack of buffers to minimize memory allocations.
template<typename T>
struct hz_stack {
    std::stack<T *> buffers;
    T *curr_buf;
    uint8_t index = 0;
    const uint8_t buffer_size = 0xFF;

    hz_stack() {
        index = 0;
        curr_buf = new T[buffer_size];
    }

    void push(T x) {
        if (index < buffer_size) {
            curr_buf[index++] = x;
        } else {
            buffers.push(curr_buf);
            curr_buf = new T[buffer_size];
            index = 0;
            curr_buf[index++] = x;
        }
    }

    void pop() {
        if (index > 1) {
            index--;
        } else {
            if (buffers.size() > 0) {
                index = buffer_size;
                curr_buf = buffers.top();
                buffers.pop();
            } else {
                index = 0;
                return;
            }
        }
    }

    T top() {
        return curr_buf[index - 1];
    }

    uint64_t size() {
        return buffers.size() * buffer_size + index;
    }

    bool empty() {
        return index == 0 && buffers.empty();
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

HZ_INLINE bin_t unaryinv_bin(std::function<uint64_t(uint64_t)> readfunc) {
    uint64_t count = 0;

    while (readfunc(1) != 0) {
        count++;
    }

    uint64_t obj = readfunc(count);
    return bin_t{.obj=obj, .n=(HZ_UINT) count};
}

#endif
