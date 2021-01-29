#ifndef HYBRIDZIP_HZRANS_H
#define HYBRIDZIP_HZRANS_H

#ifndef HZRANS_SCALE
#define HZRANS_SCALE 12
#endif

#include "hzrans64.h"
#include <functional>
#include <hzip_core/utils/stack.h>

struct u64ptr {
    uint64_t *data;
    uint64_t n;
};

struct u32ptr {
    uint32_t *data;
    uint64_t n;
};

struct hzrans64_encoder_output {
    rainman::ptr<uint32_t> data;
    uint64_t n;
};

struct hzrans64_decoder_output {
    rainman::ptr<uint64_t> data;
    uint64_t n;
};

using hz_codec_callback = std::function<void(uint64_t, const rainman::ptr<uint64_t> &)>;
using hz_cross_codec = std::function<void(
        const rainman::ptr<hzrans64_t> &state,
        const rainman::ptr<HZ_Stack<uint32_t>> &data
)>;

#endif
