#include "hzbin.h"
#include <hzip/utils/stack.h>

void hzu_encoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = HZ_NEW(hzrans64_t);
    HZ_MEM_INIT_PTR(state);

    index = 0;
    distptr = nullptr;
    cross_encoder = nullptr;
    size = buffer_size;
    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
}

void hzu_encoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzu_encoder::set_cross_encoder(hz_cross_encoder _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

u32ptr hzu_encoder::encode() {
    auto data = HZ_NEW(hz_stack<uint32_t>);

    while (index--) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, data);
        }
        hzrans64_encode_s(state, data);
    }

    hzrans64_enc_flush(state, data);

    index = 0; // reset index

    auto _data = HZ_MALLOC(uint32_t, data->size());

    for (int i = 0; !data->empty(); i++) {
        _data[i] = data->top();
        data->pop();
    }

    HZ_FREE(data);

    return u32ptr{.data = _data, .n = state->count};
}

hzu_encoder::~hzu_encoder() {
    HZ_FREE(state);
    HZ_FREE(distptr);
}

void hzu_encoder::set_size(uint64_t _size) {
    index = _size;
}

void hzu_decoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = HZ_NEW(hzrans64_t);
    HZ_MEM_INIT_PTR(state);

    size = buffer_size;
    distptr = nullptr;
    sym_override_ptr = nullptr;

    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
}

void hzu_decoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzu_decoder::set_callback(hz_codec_callback _callback) {
    callback = std::move(_callback);
}

void hzu_decoder::set_cross_encoder(hz_cross_encoder _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

u64ptr hzu_decoder::decode(uint32_t *raw, bool bypass_normalization) {
    hzrans64_dec_load_state(state, &raw);
    auto *sym = HZ_MALLOC(uint64_t, size);
    auto *dummy_stack = HZ_NEW(hz_stack<uint32_t>);

    for (int i = 0; i < size; i++) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, dummy_stack);
        }

        hzrans64_decode_s(state, distptr, &raw, sym + i, bypass_normalization);

        if (sym_override_ptr != nullptr) {
            sym[i] = *sym_override_ptr;
        }

        callback(sym[i], distptr);
    }

    HZ_FREE(dummy_stack);
    return u64ptr{.data = sym, .n = size};
}

hzu_decoder::~hzu_decoder() {
    HZ_FREE(state);
    HZ_FREE(distptr);
}

void hzu_decoder::override_symbol_ptr(uint64_t *ptr) {
    sym_override_ptr = ptr;
}
