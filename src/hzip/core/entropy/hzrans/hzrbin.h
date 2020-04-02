#ifndef HYBRIDZIP_HZRBIN_H
#define HYBRIDZIP_HZRBIN_H

#include <functional>
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
        size = buffer_size;
        if (scale > 31) scale = 31;

        hzrans64_codec_init(state, alphabet_size, scale);
        hzrans64_alloc_frame(state, buffer_size);
    }

    void setExtractionFunc(std::function<uint64_t(void)> _extract) {
        extract = _extract;
    }

    void setDistribution(uint64_t *ptr) {
        distptr = ptr;
    }

    void setCallback(hz_codec_callback _callback) {
        callback = std::move(_callback);
    }

    void setCrossEncoder(hz_cross_encoder _cross_encoder) {
        cross_encoder = std::move(_cross_encoder);
    }

    void normalize() {
        uint64_t symbol = extract();
        hzrans64_create_ftable_nf(state, distptr);
        hzrans64_add_to_seq(state, symbol, index++);
        // DEBUG START

        callback(symbol, distptr);
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

        auto __data = new uint32_t[data->size()];

        for (int i = 0; !data->empty(); i++) {
            __data[i] = data->top();
            data->pop();
        }

        return u32ptr{.data = __data, .n = state->count};
    }

    ~HZRUEncoder() {
        delete state;
        free(distptr);
    }

};

class HZRUDecoder {
private:
    hz_codec_callback callback;
    uint64_t *distptr;
    uint64_t size;
    hzrans64_t *state;

public:
    HZRUDecoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size) {
        state = new hzrans64_t;
        size = buffer_size;
        distptr = nullptr;
        if (scale > 31) scale = 31;

        hzrans64_codec_init(state, alphabet_size, scale);
        hzrans64_alloc_frame(state, buffer_size);
    }

    void setDistribution(uint64_t *ptr) {
        distptr = ptr;
    }

    void setCallback(hz_codec_callback _callback) {
        callback = _callback;
    }

    u64ptr decode(uint32_t *raw) {
        hzrans64_dec_load_state(state, &raw);
        auto *sym = new uint64_t[size];
        for (int i = 0; i < size; i++) {
            hzrans64_decode_s(state, distptr, i, &raw, sym + i);
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
