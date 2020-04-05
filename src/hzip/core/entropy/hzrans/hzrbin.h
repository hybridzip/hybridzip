#ifndef HYBRIDZIP_HZRBIN_H
#define HYBRIDZIP_HZRBIN_H

#include <functional>
#include <utility>
#include "hzrans.h"


class HZRUEncoder {
private:
    std::function<uint64_t(void)> extract;
    hz_cross_encoder cross_encoder;
    hz_codec_callback callback;
    uint64_t *distptr;
    uint64_t size;
    int64_t index;
    hzrans64_t *state;

public:
    HZRUEncoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
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

    void set_extractor(std::function<uint64_t(void)> _extract) {
        extract = std::move(_extract);
    }

    void set_distribution(uint64_t *ptr) {
        distptr = ptr;
    }

    void set_callback(hz_codec_callback _callback) {
        callback = std::move(_callback);
    }

    void set_cross_encoder(hz_cross_encoder _cross_encoder) {
        cross_encoder = std::move(_cross_encoder);
    }

    void normalize(bool bypass_normalization=false) {
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

    u32ptr encode() {
        auto data = new light_stack<uint32_t>();

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

    ~HZRUEncoder() {
        delete state;
        free(distptr);
    }

};

class HZRUDecoder {
private:
    hz_codec_callback callback;
    hz_cross_encoder cross_encoder;
    std::function<uint64_t()> symbol_callback;
    uint64_t *distptr;
    uint64_t size;
    hzrans64_t *state;

public:
    HZRUDecoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
        state = new hzrans64_t;
        size = buffer_size;
        distptr = nullptr;
        symbol_callback = nullptr;
        if (scale > 31) scale = 31;

        hzrans64_codec_init(state, alphabet_size, scale);
        hzrans64_alloc_frame(state, buffer_size);
    }

    void set_distribution(uint64_t *ptr) {
        distptr = ptr;
    }

    void set_callback(hz_codec_callback _callback) {
        callback = std::move(_callback);
    }

    void set_symbol_callback(std::function<uint64_t()> _symbol_callback) {
        symbol_callback = std::move(_symbol_callback);
    }

    void set_cross_encoder(hz_cross_encoder _cross_encoder) {
        cross_encoder = std::move(_cross_encoder);
    }

    u64ptr decode(uint32_t *raw, bool bypass_normalization=false) {
        hzrans64_dec_load_state(state, &raw);
        auto *sym = new uint64_t[size];
        auto *dummy_stack = new light_stack<uint32_t>();
        for (int i = 0; i < size; i++) {
            if (cross_encoder != nullptr) {
                cross_encoder(state, dummy_stack);
            }
            hzrans64_decode_s(state, distptr, i, &raw, sym + i, bypass_normalization, symbol_callback);
            callback(sym[i], distptr);
        }
        return u64ptr{.data = sym, .n = size};
    }

    ~HZRUDecoder() {
        delete state;
        free(distptr);
    }

};

#endif
