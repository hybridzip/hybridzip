#ifndef HYBRIDZIP_STACK_H
#define HYBRIDZIP_STACK_H

#include <cstdint>

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

#endif
