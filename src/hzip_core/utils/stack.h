#ifndef HYBRIDZIP_STACK_H
#define HYBRIDZIP_STACK_H

#include <cstdint>
#include <rainman/rainman.h>

// A light-weight efficient stack that maintains a stack of buffers to minimize memory allocations.
template<typename Type>
struct HZ_Stack : public rainman::Allocator {
    struct HZ_Buffer : private rainman::Allocator {
        Type *data;
        HZ_Buffer *next;
        HZ_Buffer *prev;

        HZ_Buffer(
                uint64_t size,
                const rainman::Allocator &allocator = rainman::Allocator()
        ) : rainman::Allocator(allocator) {
            data = rnew<Type>(size);
            next = nullptr;
            prev = nullptr;
        }

        void destroy() {
            rfree(data);
        }
    };

    HZ_Buffer *buffer;
    const uint64_t bufsize = 0xFFF;
    uint64_t index;

    HZ_Stack(const rainman::Allocator &allocator = rainman::Allocator()) : rainman::Allocator(allocator) {
        buffer = nullptr;
        index = 0;
    }

    void push(Type x) {
        if (buffer == nullptr) {
            buffer = rnew<HZ_Buffer>(1, bufsize, *this);
        }

        if (index < bufsize) {
            buffer->data[index++] = x;
        } else {
            buffer->next = rnew<HZ_Buffer>(1, bufsize, *this);
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

    Type top() {
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

    ~HZ_Stack() {
        while (buffer != nullptr) {
            auto tmp = buffer;
            buffer = buffer->prev;

            tmp->destroy();
            rfree(tmp);
        }
    }
};

#endif
