//
// Created by Vishaal Selvaraj on 04-12-2019.
//

#ifndef HYBRIDZIP_HZRANS_H
#define HYBRIDZIP_HZRANS_H
#ifndef HZRANS_USE_AVX
#define HZRANS_USE_AVX 0
#endif

#ifndef HZRANS_SCALE
#define HZRANS_SCALE 12
#endif

#if HZRANS_USE_AVX
#include "hzrans64avx.h"
#else

#include "hzrans64.h"

#endif

#include <functional>

struct u64ptr {
    uint64_t *data;
    uint64_t n;
};

struct u32ptr {
    uint32_t *data;
    uint64_t n;
};

typedef std::function<void(uint64_t, uint64_t *)> hz_codec_callback;
typedef std::function<void(hzrans64_t *, light_stack<uint32_t> *data)> hz_cross_encoder;

#endif //HYBRIDZIP_HZRANS_H
