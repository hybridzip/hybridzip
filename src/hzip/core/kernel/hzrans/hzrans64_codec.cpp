#include "hzrans64_codec.h"
#include <hzip/utils/stack.h>

void hzrans64_encoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = rnew(hzrans64_t);
    rinitptr(state);

    index = 0;
    distptr = nullptr;
    cross_encoder = nullptr;
    size = buffer_size;
    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
}

void hzrans64_encoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzrans64_encoder::set_cross_encoder(hz_cross_codec _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

u32ptr hzrans64_encoder::encode() {
    auto data = rnew(HZ_Stack<uint32_t>);

    while (index--) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, data);
        }
        hzrans64_encode_s(state, data);
    }

    hzrans64_enc_flush(state, data);

    index = 0; // reset index

    auto _data = rmalloc(uint32_t, data->size());

    for (int i = 0; !data->empty(); i++) {
        _data[i] = data->top();
        data->pop();
    }

    rfree(data);

    return u32ptr{.data = _data, .n = state->count};
}

hzrans64_encoder::~hzrans64_encoder() {
    hzrans64_destroy(state);
    rfree(distptr);
}

void hzrans64_encoder::set_size(uint64_t _size) {
    index = _size;
}

void hzrans64_decoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = rnew(hzrans64_t);
    rinitptr(state);

    size = buffer_size;
    distptr = nullptr;
    sym_override_ptr = nullptr;

    if (scale > 31) scale = 31;

    hzrans64_codec_init(state, alphabet_size, scale);
}

void hzrans64_decoder::set_distribution(uint64_t *ptr) {
    distptr = ptr;
}

void hzrans64_decoder::set_cross_decoder(hz_cross_codec _cross_decoder) {
    cross_decoder = std::move(_cross_decoder);
}

u64ptr hzrans64_decoder::decode(uint32_t *raw) {
    hzrans64_dec_load_state(state, &raw);
    auto *sym = rmalloc(uint64_t, size);
    auto *dummy_stack = rnew(HZ_Stack<uint32_t>);

    for (int i = 0; i < size; i++) {
        if (cross_decoder != nullptr) {
            cross_decoder(state, dummy_stack);
        }

        hzrans64_decode_s(state, &raw);

        if (sym_override_ptr != nullptr) {
            sym[i] = *sym_override_ptr;
        }
    }

    dummy_stack->destroy();
    rfree(dummy_stack);
    return u64ptr{.data = sym, .n = size};
}

hzrans64_decoder::~hzrans64_decoder() {
    hzrans64_destroy(state);
    rfree(distptr);
}

void hzrans64_decoder::override_symbol_ptr(uint64_t *ptr) {
    sym_override_ptr = ptr;
}
