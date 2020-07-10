#include "hzbin.h"
#include <hzip/utils/stack.h>

hzu_encoder::hzu_encoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = new hzrans64_t;
    index = 0;
    distptr = nullptr;
    callback = nullptr;
    cross_encoder = nullptr;
    size = buffer_size;
    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
    hzrans64_alloc_frame(state, buffer_size);
}

void hzu_encoder::set_extractor(std::function<uint64_t(void)> _extract) {
    extract = std::move(_extract);
}

void hzu_encoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzu_encoder::set_callback(hz_codec_callback _callback) {
    callback = std::move(_callback);
}

void hzu_encoder::set_cross_encoder(hz_cross_encoder _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

void hzu_encoder::normalize(bool bypass_normalization) {
    uint64_t symbol = extract();
    if (!bypass_normalization) {
        hzrans64_create_ftable_nf(state, distptr);
        hzrans64_add_to_seq(state, symbol, index);
    }
    index++;
    if (callback != nullptr) {
        callback(symbol, distptr);
    }
}

u32ptr hzu_encoder::encode() {
    auto data = new hz_stack<uint32_t>();

    while (index--) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, data);
        }
        hzrans64_encode_s(state, index, data);
    }

    hzrans64_enc_flush(state, data);

    index = 0; // reset index

    auto _data = new uint32_t[data->size()];

    for (int i = 0; !data->empty(); i++) {
        _data[i] = data->top();
        data->pop();
    }

    return u32ptr{.data = _data, .n = state->count};
}

hzu_encoder::~hzu_encoder() {
    delete state;
    free(distptr);
}


hzu_decoder::hzu_decoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = new hzrans64_t;
    size = buffer_size;
    distptr = nullptr;
    symbol_callback = nullptr;
    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
    hzrans64_alloc_frame(state, buffer_size);
}

void hzu_decoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzu_decoder::set_callback(hz_codec_callback _callback) {
    callback = std::move(_callback);
}

void hzu_decoder::set_symbol_callback(std::function<uint64_t()> _symbol_callback) {
    symbol_callback = std::move(_symbol_callback);
}

void hzu_decoder::set_cross_encoder(hz_cross_encoder _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

u64ptr hzu_decoder::decode(uint32_t *raw, bool bypass_normalization) {
    hzrans64_dec_load_state(state, &raw);
    auto *sym = new uint64_t[size];
    auto *dummy_stack = new hz_stack<uint32_t>();
    for (int i = 0; i < size; i++) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, dummy_stack);
        }
        hzrans64_decode_s(state, distptr, i, &raw, sym + i, bypass_normalization, symbol_callback);
        callback(sym[i], distptr);
    }
    return u64ptr{.data = sym, .n = size};
}

hzu_decoder::~hzu_decoder() {
    delete state;
    free(distptr);
}
