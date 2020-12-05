#ifndef HYBRIDZIP_STACK_H
#define HYBRIDZIP_STACK_H

#include <cstdint>

// A light-weight efficient stack that maintains a stack of buffers to minimize memory allocations.
template<typename T>
struct HZ_Stack {
    struct HZ_Buffer {
        T *data;
        HZ_Buffer *next;
        HZ_Buffer *prev;

        HZ_Buffer(uint64_t size) {
            data = new T[size];
            next = nullptr;
            prev = nullptr;
        }

        void destroy() {
            delete[] data;
        }
    };

    HZ_Buffer *buffer;
    const uint64_t bufsize = 0xFFF;
    uint64_t index;

    HZ_Stack() {
        buffer = nullptr;
        index = 0;
    }

    void push(T x) {
        if (buffer == nullptr) {
            buffer = new HZ_Buffer(bufsize);
        }

        if (index < bufsize) {
            buffer->data[index++] = x;
        } else {
            buffer->next = new HZ_Buffer(bufsize);
            buffer->next->prev = buffer;
            buffer = buffer->next;
            index = 0;
            buffer->data[index++] = x;
        }
    }

    void pop() {
        if (buffer == nullptr) {
            return;
        }

        if (index > 1) {
            index--;
        } else {
            index = bufsize;
            auto tmp = buffer;
            buffer = buffer->prev;

            tmp->destroy();
            delete tmp;
        }
    }

    T top() {
        return buffer->data[index - 1];
    }

    uint64_t size() {
        uint64_t count = index;

        if (buffer != nullptr) {
            auto tmp = buffer->prev;

            while (tmp != nullptr) {
                tmp = tmp->prev;
                count += bufsize;
            }
        }

        return count;
    }

    bool empty() {
        return buffer == nullptr;
    }

    void destroy() {
        while (buffer != nullptr) {
            auto *tmp = buffer;
            buffer = buffer->prev;

            tmp->destroy();
            delete tmp;
        }
    }
};

#endif
