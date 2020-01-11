#ifndef HYBRIDZIP_UNARY_H
#define HYBRIDZIP_UNARY_H

#include <functional>
#include <cstdint>
#include "common.h"

uint64_t u64log2(uint64_t n) {
    if (n == 0) return 0;
    uint64_t count = 0;
    while (n > 0) {
        n >>= 1;
        count++;
    }
    return --count;
}

HZIP_FORCED_INLINE bin_t unarypx_bin(uint64_t n) {
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
    return bin_t{.obj=num, .n=(HZIP_UINT) (1 + ((1 + lg2) << 1))};
}

HZIP_FORCED_INLINE bin_t unaryinv_bin(std::function<uint64_t(uint64_t)> readfunc) {
    uint64_t count = 0;

    while (readfunc(1) != 0) {
        count++;
    }

    uint64_t obj = readfunc(count);
    return bin_t{.obj=obj, .n=(HZIP_UINT) count};
}

#endif
