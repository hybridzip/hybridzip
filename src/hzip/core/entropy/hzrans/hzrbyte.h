//
// Created by Vishaal Selvaraj on 30-11-2019.
// OOP Wrapper for byte-based rans64 encoder.
//

#ifndef HYBRIDZIP_HZRBYTE_H
#define HYBRIDZIP_HZRBYTE_H

#include <functional>
#include "hzrans.h"






class hzrByteEncoder {
private:
    uint64_t *data;
    uint64_t *distptr;
    int64_t index;
    uint64_t buffer_size;
    hzrans64_t *state;
public:
    hzrByteEncoder(uint64_t bufsize, uint16_t scale) {
        data = new uint64_t[bufsize];
        data += bufsize;
        index = 0;
        distptr = nullptr;
        buffer_size = bufsize;

        state = new hzrans64_t;
        if (scale > 31) scale = 31;
        hzrans64_codec_init(state, 0x100, scale);
        hzrans64_alloc_frame(state, bufsize);
    }

    void setDistribution(uint64_t *dist) {
        distptr = dist;
    }

    void normalize(uint64_t obj, hz_codec_callback callback) {
        hzrans64_create_ftable_nf(state, distptr);
        hzrans64_add_to_seq(state, obj, index++);
        callback(obj, distptr);
    }

    u64ptr encodeBytes() {
        while (index--)
            hzrans64_encode_s(state, index, &data);
        hzrans64_enc_flush(state, &data);

        index = 0; // reset index

        return u64ptr{.data = data, .n = state->count};
    }
};

class hzrByteDecoder {
private:
    uint64_t buffer_size;
    uint64_t *distptr;
    hzrans64_t *state;
public:
    hzrByteDecoder(uint64_t bufsize, uint16_t scale) {
        buffer_size = bufsize;
        distptr = nullptr;

        state = new hzrans64_t;
        if (scale > 31) scale = 31;
        hzrans64_codec_init(state, 0x100, scale);
        hzrans64_alloc_frame(state, bufsize);
    }

    void setDistribution(uint64_t *dist) {
        distptr = dist;
    }

    u64ptr decodeBytes(uint64_t *bytes, hz_codec_callback callback) {
        hzrans64_dec_load_state(state, &bytes);
        uint64_t *sym = new uint64_t[buffer_size];
        for (int i = 0; i < buffer_size; i++) {
            hzrans64_decode_s(state, distptr, i, &bytes, sym + i);

            callback(sym[i], distptr);
        }
        return u64ptr {.data = sym, .n = buffer_size};
    }
};

#endif
