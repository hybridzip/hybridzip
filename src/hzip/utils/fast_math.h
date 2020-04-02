#ifndef HYBRIDZIP_FAST_MATH_H
#define HYBRIDZIP_FAST_MATH_H

#include <cstdint>

inline uint8_t fast_log2(uint64_t x) {
    uint8_t count = 0;
    while (x != 0) {
        x >>= 1;
        count++;
    }
    return count;
}

#endif
