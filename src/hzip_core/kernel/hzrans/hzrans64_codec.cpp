#include "hzrans64_codec.h"
#include <hzip_core/utils/stack.h>

void hzrans64_encoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = rainman::ptr<hzrans64_t>();

    index = 0;
    cross_encoder = nullptr;
    size = buffer_size;
    if (scale > 31) scale = 31;

    hzrans64_codec_init(state.pointer(), alphabet_size, scale);
}

void hzrans64_encoder::set_distribution(const rainman::ptr<uint64_t> &ptr) {
    distptr = ptr;
}

void hzrans64_encoder::set_cross_encoder(hz_cross_codec _cross_encoder) {
    cross_encoder = std::move(_cross_encoder);
}

hzrans64_encoder_output hzrans64_encoder::encode() {
    auto data = rainman::ptr<HZ_Stack<uint32_t>>();

    while (index--) {
        if (cross_encoder != nullptr) {
            cross_encoder(state, data);
        }
        hzrans64_encode_s(state.pointer(), data.pointer());
    }

    hzrans64_enc_flush(state.pointer(), data.pointer());

    index = 0; // reset index

    auto _data = rainman::ptr<uint32_t>(data->size());

    for (int i = 0; !data->empty(); i++) {
        _data[i] = data->top();
        data->pop();
    }

    return hzrans64_encoder_output{.data = _data, .n = state->count};
}

hzrans64_encoder::~hzrans64_encoder() {
    hzrans64_destroy(state.pointer());
}

void hzrans64_encoder::set_size(uint64_t _size) {
    index = _size;
}

void hzrans64_decoder::set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
    state = rainman::ptr<hzrans64_t>();

    size = buffer_size;

    if (scale > 31) scale = 31;

    hzrans64_codec_init(state.pointer(), alphabet_size, scale);
}

void hzrans64_decoder::set_distribution(const rainman::ptr<uint64_t> &ptr) {
    distptr = ptr;
}

void hzrans64_decoder::set_cross_decoder(hz_cross_codec _cross_decoder) {
    cross_decoder = std::move(_cross_decoder);
}

hzrans64_decoder_output hzrans64_decoder::decode(rainman::ptr<uint32_t> &raw) {
    hzrans64_dec_load_state(state.pointer(), raw);
    auto sym = rainman::ptr<uint64_t>(size);
    auto dummy_stack = rainman::ptr<HZ_Stack<uint32_t>>();

    for (int i = 0; i < size; i++) {
        if (cross_decoder != nullptr) {
            cross_decoder(state, dummy_stack);
        }

        hzrans64_decode_s(state.pointer(), raw);

        if (sym_override_ptr.is_some()) {
            sym[i] = *sym_override_ptr.inner();
        }
    }

    return hzrans64_decoder_output{.data = sym, .n = size};
}

hzrans64_decoder::~hzrans64_decoder() {
    hzrans64_destroy(state.pointer());
}

void hzrans64_decoder::override_symbol_ptr(const rainman::ptr<uint64_t> &ptr) {
    sym_override_ptr = ptr;
}
