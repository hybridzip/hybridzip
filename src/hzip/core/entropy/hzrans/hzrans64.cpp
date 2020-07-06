#include "hzrans64.h"


 void hzrans64_alloc_frame(hzrans64_t *state, uint64_t block_size) {
    state->ls = new uint64_t[block_size];
    state->bs = new uint64_t[block_size];
}

 uint64_t hzrans64_bs(hzrans64_t *state, uint16_t symbol) {
    uint64_t i = 0;
    uint64_t cumsum = 0;
    while (i <= symbol) {
        cumsum += state->ftable[i++];
    }
    return cumsum;
}


 uint64_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs) {
    uint64_t i, cumsum = 0;
    for (i = 0; i < state->size; i++) {
        if (cumsum > bs)
            break;
        cumsum += state->ftable[i];
    }
    return --i;
}

 void hzrans64_set_alphabet_size(hzrans64_t *state, uint64_t size) {
    state->size = size;
}

 void hzrans64_codec_init(hzrans64_t *state, uint64_t size, uint64_t scale) {
    state->x = state->lower_bound;
    state->size = size;
    state->ftable = (uint64_t *) malloc(sizeof(uint64_t) * size);
    state->scale = scale;
    state->up_prefix = (state->lower_bound >> scale) << 32;
    state->mask = (1ull << scale) - 1;
    state->count = 0;
}

 void hzrans64_encode_s(hzrans64_t *state, uint64_t index, hz_stack<uint32_t> *data) {

    uint64_t x = state->x;
    uint64_t ls = state->ls[index];
    uint64_t bs = state->bs[index];
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


 void
hzrans64_add_to_seq(hzrans64_t *state, uint64_t symbol, uint64_t index) {
    state->bs[index] = 0;
    state->ls[index] = state->ftable[symbol];

    for (int i = 0; i < symbol; i++) {
        state->bs[index] += state->ftable[i];
    }
}

 void hzrans64_enc_flush(hzrans64_t *state, hz_stack<uint32_t> *data) {
    data->push(state->x >> 32);
    data->push(state->x >> 0);
    state->count += 2;
}

 void hzrans64_dec_load_state(hzrans64_t *state, uint32_t **data) {
    uint64_t x;
    x = (uint64_t) ((*data)[0]) << 0;
    x |= (uint64_t) ((*data)[1]) << 32;
    *data += 2;
    state->x = x;
}

 void
hzrans64_decode_s(hzrans64_t *state, uint64_t *_ls, uint64_t index, uint32_t **data, uint64_t *sym,
                  bool bypass_normalization, std::function<uint64_t()> symbol_callback) {
    uint64_t x = state->x;

    if (!bypass_normalization) {
        hzrans64_create_ftable_nf(state, _ls);
        uint64_t symbol = hzrans64_inv_bs(state, x & state->mask);
        *sym = symbol;
        hzrans64_add_to_seq(state, symbol, index);
    }
    if (symbol_callback != nullptr) {
        *sym = symbol_callback();
    }

    uint64_t ls = state->ls[index];
    uint64_t bs = state->bs[index];

    x = (ls * (x >> state->scale)) + (x & state->mask) - bs;
    if (x < state->lower_bound) {
        x = (x << 32) | **data;
        *data += 1;
    }

    state->x = x;
}