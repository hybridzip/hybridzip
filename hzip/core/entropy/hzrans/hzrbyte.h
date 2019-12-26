//
// Created by Vishaal Selvaraj on 30-11-2019.
// OOP Wrapper for byte-based rans64 encoder.
//

#ifndef HYBRIDZIP_HZRBYTE_H
#define HYBRIDZIP_HZRBYTE_H

#include <functional>
#include "hzrans64.h"


typedef std::function<void(uint8_t, int32_t*)> enc_callback;

struct u32ptr {
    uint32_t *data;
    uint64_t n;
};

struct u8ptr {
    uint8_t *data;
    uint64_t n;
};

class hzrByteEncoder {
private:
    uint32_t *data;
    int32_t *distptr;
    int64_t index;
    hzrans64_t *state;
public:
    hzrByteEncoder(uint64_t bufsize, uint16_t scale) {
        data = new uint32_t[bufsize];
        data += bufsize;
        index = 0;
        distptr = nullptr;

        state = new hzrans64_t;
        if (scale > 31) scale = 31;
        hzrans64_codec_init(state, 0x100, scale);
        hzrans64_alloc_frame(state, bufsize);
    }

    void setDistribution(int32_t *dist) {
        distptr = dist;
    }

    void normalize(uint8_t byte, enc_callback callback) {
        hzrans64_create_ftable_nf(state, distptr);
        hzrans64_add_to_seq(state, byte, index++);
        callback((uint8_t)byte, (int32_t*)distptr);
    }

    u32ptr encodeBytes() {
        while (index--)
            hzrans64_encode_s(state, index, &data);
        hzrans64_enc_flush(state, &data);

        index = 0; // reset index

        return u32ptr{.data = data, .n = state->count};
    }
};

class hzrByteDecoder {
private:
    uint64_t buffer_size;
    int32_t *distptr;
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

    void setDistribution(int32_t *dist) {
        distptr = dist;
    }

    u8ptr decodeBytes(uint32_t *bytes, void (*callback)(uint8_t symbol)) {
        hzrans64_dec_load_state(state, &bytes);
        uint8_t *sym = new uint8_t[buffer_size];
        for (int i = 0; i < buffer_size; i++) {
            hzrans64_decode_s(state, distptr, i, &bytes, sym + i);
            callback(sym[i]);
        }
        return u8ptr {.data = sym, .n = buffer_size};
    }

    u8ptr decodeBytes(uint32_t *bytes, std::function <void(uint8_t)> callback) {
        hzrans64_dec_load_state(state, &bytes);
        uint8_t *sym = new uint8_t[buffer_size];
        for (int i = 0; i < buffer_size; i++) {
            hzrans64_decode_s(state, distptr, i, &bytes, sym + i);
            callback(sym[i]);
        }
        return u8ptr {.data = sym, .n = buffer_size};
    }
};

#endif
