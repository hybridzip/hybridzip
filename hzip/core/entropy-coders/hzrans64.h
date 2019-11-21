#ifndef HYBRIDZIP_HZRANS64_H
#define HYBRIDZIP_HZRANS64_H

#include "../../other/platform.h"

struct hzrans64_t {
    uint64_t x;
    uint16_t size;
    uint64_t *ftable;
    uint8_t scale;
    const unsigned long long lower_bound = 1ull << 31;
    unsigned long long up_prefix;
};


HZIP_FORCED_INLINE uint64_t hzrans64_bs(hzrans64_t *state, uint16_t symbol) {
    uint16_t i = 0;
    uint64_t cumsum = 0;
    while (i <= symbol) {
        cumsum += state->ftable[i++];
    }
    return cumsum;
}


HZIP_FORCED_INLINE uint16_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs) {
    uint64_t cumsum = 0;
    uint16_t i;
    for (i = 0; cumsum < bs; cumsum += state->ftable[i], i++);
    return --i;
}


HZIP_FORCED_INLINE void hzrans64_encoder_init(hzrans64_t *state, uint16_t size, uint8_t scale) {
    state->x = 1;
    state->size = size;
    state->ftable = (uint64_t *) malloc(sizeof(uint64_t) * size);
    state->scale = scale;
    state->up_prefix = (state->lower_bound >> scale) << 32;
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


#endif
