#include "hzrans64.h"
#include <hzip_core/utils/stack.h>

uint64_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs) {
    uint64_t i, cumsum = 0;
    for (i = 0; i < state->size; i++) {
        if (cumsum > bs)
            break;
        cumsum += state->ftable[i];
    }
    return --i;
}

void hzrans64_codec_init(hzrans64_t *state, uint64_t size, uint64_t scale) {
    state->x = state->lower_bound;
    state->size = size;
    state->ftable = rglobalmgr.r_malloc<uint64_t>(size);
    state->scale = scale;
    state->up_prefix = (state->lower_bound >> scale) << 32;
    state->mask = (1ull << scale) - 1;
    state->count = 0;
}

void hzrans64_encode_s(hzrans64_t *state, HZ_Stack<uint32_t> *data) {
    uint64_t x = state->x;
    uint64_t ls = state->ls;
    uint64_t bs = state->bs;
    uint64_t upper_bound = ls * state->up_prefix;

    if (x >= upper_bound) {
        data->push(x);
        x >>= 32;
        state->count++;
    }

    x = ((x / ls) << state->scale) + bs + (x % ls);
    state->x = x;
}


void hzrans64_create_ftable_nf(hzrans64_t *state, uint64_t *freq) {
    uint64_t sum = state->size;
    for (int i = 0; i < state->size; i++) {
        sum += freq[i];
    }
    // use three-layered normalization.
    uint64_t ssum = 0;
    uint64_t mul_factor = (1ull << state->scale) - state->size;


    for (int i = 0; i < state->size; i++) {
        uint64_t value = 1 + (freq[i] + 1) * mul_factor / sum;
        ssum += value - 1;
        state->ftable[i] = value;
    }

    //disperse residues.
    ssum = mul_factor - ssum;
    for (int i = 0; ssum > 0; i++, ssum--)
        state->ftable[i]++;
}


void hzrans64_add_to_seq(hzrans64_t *state, uint64_t symbol) {
    state->bs = 0;
    state->ls = state->ftable[symbol];

    for (int i = 0; i < symbol; i++) {
        state->bs += state->ftable[i];
    }
}

void hzrans64_enc_flush(hzrans64_t *state, HZ_Stack<uint32_t> *data) {
    data->push(state->x >> 32);
    data->push(state->x >> 0);
    state->count += 2;
}

void hzrans64_dec_load_state(hzrans64_t *state, rainman::ptr<uint32_t> &data) {
    uint64_t x;
    x = (uint64_t) (data[0]) << 0;
    x |= (uint64_t) (data[1]) << 32;
    data += 2;
    state->x = x;
}

void hzrans64_decode_s(hzrans64_t *state, rainman::ptr<uint32_t> &data) {
    uint64_t x = state->x;
    uint64_t ls = state->ls;
    uint64_t bs = state->bs;

    x = (ls * (x >> state->scale)) + (x & state->mask) - bs;
    if (x < state->lower_bound) {
        x = (x << 32) | *data;
        data += 1;
    }

    state->x = x;
}

void hzrans64_destroy(hzrans64_t *state) {
    rglobalmgr.r_free(state->ftable);
}
