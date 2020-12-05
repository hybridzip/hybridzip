#ifndef HYBRIDZIP_HZRANS64_CODEC_H
#define HYBRIDZIP_HZRANS64_CODEC_H

#include <functional>
#include <utility>
#include <rainman/rainman.h>
#include "hzrans.h"


class hzrans64_encoder : public rainman::context {
private:
    hz_cross_codec cross_encoder;
    uint64_t *distptr{};
    uint64_t size{};
    int64_t index{};
    hzrans64_t *state{};

public:
    hzrans64_encoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void set_cross_encoder(hz_cross_codec _cross_encoder);

    void set_size(uint64_t _size);

    u32ptr encode();

    ~hzrans64_encoder();

};

class hzrans64_decoder : public rainman::context {
private:
    hz_codec_callback callback;
    hz_cross_codec cross_decoder;
    uint64_t *sym_override_ptr = nullptr;
    uint64_t *distptr = nullptr;
    uint64_t size = 0;
    hzrans64_t *state = nullptr;

public:
    hzrans64_decoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void override_symbol_ptr(uint64_t *ptr);

    void set_cross_decoder(hz_cross_codec _cross_decoder);

    u64ptr decode(uint32_t *raw);

    ~hzrans64_decoder();

};

#endif
