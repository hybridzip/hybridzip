#ifndef HYBRIDZIP_LIGHT_STACK_H
#define HYBRIDZIP_LIGHT_STACK_H

#include <cstdint>
#include <stack>


// A light-weight efficient stack that maintains a stack of buffers to minimize memory allocations.
template<typename T>
struct light_stack {
    std::stack<T*> buffers;
    T *curr_buf;
    uint8_t index = 0;
    const uint8_t buffer_size = 0xFF;

    light_stack() {
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

#endif
