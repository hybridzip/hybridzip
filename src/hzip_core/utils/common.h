#ifndef HYBRIDZIP_COMMON_H
#define HYBRIDZIP_COMMON_H

#include <hzip_core/utils/platform.h>

typedef struct bin_t {
    HZ_SIZE_T obj;
    HZ_UINT n;
} bin_t;

template<typename T>
struct option_t {
    T x;
    bool is_valid{};

    explicit option_t(T x, bool v = true) {
        this->x = x;
        this->is_valid = v;
    }

    T get() {
        return x;
    }
};

#endif





