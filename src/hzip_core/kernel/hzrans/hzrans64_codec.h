#ifndef HYBRIDZIP_HZRANS64_CODEC_H
#define HYBRIDZIP_HZRANS64_CODEC_H

#include <functional>
#include <utility>
#include <rainman/rainman.h>
#include "hzrans.h"


class hzrans64_encoder {
private:
    hz_cross_codec cross_encoder{};
    rainman::ptr<uint64_t> distptr{};
    uint64_t size{};
    int64_t index{};
    rainman::ptr<hzrans64_t> state{};

public:
    hzrans64_encoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(const rainman::ptr<uint64_t> &ptr);

    void set_cross_encoder(hz_cross_codec _cross_encoder);

    void set_size(uint64_t _size);

    hzrans64_encoder_output encode();

    ~hzrans64_encoder();

};

class hzrans64_decoder {
private:
    hz_codec_callback callback;
    hz_cross_codec cross_decoder;
    rainman::option<rainman::ptr<uint64_t>> sym_override_ptr{};
    rainman::ptr<uint64_t> distptr{};
    uint64_t size = 0;
    rainman::ptr<hzrans64_t> state{};

public:
    hzrans64_decoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(const rainman::ptr<uint64_t> &ptr);

    void override_symbol_ptr(const rainman::ptr<uint64_t> &ptr);

    void set_cross_decoder(hz_cross_codec _cross_decoder);

    hzrans64_decoder_output decode(rainman::ptr<uint32_t> &raw);

    ~hzrans64_decoder();

};

#endif
