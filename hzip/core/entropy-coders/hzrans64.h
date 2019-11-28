#ifndef HYBRIDZIP_HZRANS64_H
#define HYBRIDZIP_HZRANS64_H
#include <cstdint>
#include <malloc.h>
#include "../../other/platform.h"

struct hzrans64_t {
    uint64_t x;
    uint16_t size;
    uint64_t *ftable;
    uint8_t scale;
    const uint64_t lower_bound = 1ull << 31;
    uint64_t up_prefix;
    uint64_t mask;
    uint64_t *ls;
    uint64_t *bs;
};

HZIP_FORCED_INLINE void hzrans64_alloc_frame(hzrans64_t *state, uint64_t block_size) {
    state->ls = new uint64_t[block_size];
    state->bs = new uint64_t[block_size];
}

HZIP_FORCED_INLINE uint64_t hzrans64_bs(hzrans64_t *state, uint16_t symbol) {
    uint16_t i = 0;
    uint64_t cumsum = 0;
    while (i <= symbol) {
        cumsum += state->ftable[i++];
    }
    return cumsum;
}

HZIP_FORCED_INLINE uint16_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs) {
    uint64_t i, cumsum = 0;
    for (i = 0; cumsum < bs; cumsum += state->ftable[i], i++);
    return --i;
}

HZIP_FORCED_INLINE void hzrans64_codec_init(hzrans64_t *state, uint16_t size, uint8_t scale) {
    state->x = 1;
    state->size = size;
    state->ftable = (uint64_t *) malloc(sizeof(uint64_t) * size);
    state->scale = scale;
    state->up_prefix = (state->lower_bound >> scale) << 32;
    state->mask = (1ull << scale) - 1;
}

HZIP_FORCED_INLINE void hzrans64_encode(hzrans64_t *state, uint16_t symbol, uint32_t **data) {
    uint64_t x = state->x;
    uint64_t freq = state->ftable[symbol];
    uint64_t upper_bound = state->up_prefix * freq;

    if (x >= upper_bound) {
        *data -= 1;
        **data = (uint32_t) x;
        x >>= 32;
    }
    state->x = (x << state->scale) / freq + hzrans64_bs(state, symbol) + (x % freq);
}

HZIP_FORCED_INLINE void hzrans64_encode_s(hzrans64_t *state, uint64_t index, uint32_t **data) {
    uint64_t x = state->x;
    uint64_t ls = state->ls[index];
    uint64_t bs = state->bs[index];
    uint64_t upper_bound = ls * state->up_prefix;
    x = (x << state->scale) / ls + bs + (x % ls);

    if (x >= upper_bound) {
        *data -= 1;
        **data = (uint32_t) x;
        x >>= 32;
    }

    state->x = x;
}

HZIP_FORCED_INLINE void hzrans64_normalize(hzrans64_t *state, uint8_t symbol, uint64_t index, uint64_t *freq, uint16_t len) {
    uint64_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += freq[i];
    }

    state->ls[index] = __max((freq[symbol] << state->scale) / sum, 1);
    state->bs[index] = 0;
    for (int i = 0; i <= symbol; i++) {
        uint64_t value = __max((freq[i] << state->scale) / sum,1);
        state->bs[index] += value;
    }

    if (symbol == 0xff) {
        state->bs[index] = (1ull << state->scale);
    }
}

HZIP_FORCED_INLINE void hzrans64_enc_flush(hzrans64_t *state, uint32_t **data) {
    *data -= 2;
    (*data)[0] = (uint32_t) (state->x >> 32);
    (*data)[1] = (uint32_t) state->x;
}

//todo: encoder-decoder-compat with model.
HZIP_FORCED_INLINE void hzrans64_dec_init(hzrans64_t *state, uint16_t size, uint8_t scale) {
    state->x = 1;
    state->size = size;
    state->ftable = (uint64_t *) malloc(sizeof(uint64_t) * size);
    state->scale = scale;
    state->up_prefix = (state->lower_bound >> scale) << 32;
}

HZIP_FORCED_INLINE void hzrans64_decode(hzrans64_t *state, uint16_t symbol, uint8_t **data) {
    uint64_t x = state->x;
    uint64_t freq = state->ftable[symbol];
    uint64_t bs = hzrans64_bs(state, symbol);
    x = freq * (x >> state->scale) + (x & state->mask) - bs;

    if (x < state->lower_bound) {
        x = (x << 32) | **data;
        *data += 1;
    }

    state->x = x;
}

#endif
