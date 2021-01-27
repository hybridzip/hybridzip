//
// Created by Vishaal Selvaraj on 04-12-2019.
//

#ifndef HYBRIDZIP_HZRANS_H
#define HYBRIDZIP_HZRANS_H

#ifndef HZRANS_SCALE
#define HZRANS_SCALE 12
#endif

#include "hzrans64.h"
#include <hzip/utils/stack.h>
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
typedef std::function<void(hzrans64_t *, HZ_Stack<uint32_t> *data)> hz_cross_codec;

#endif
