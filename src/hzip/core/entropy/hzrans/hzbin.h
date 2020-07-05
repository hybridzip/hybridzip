#ifndef HYBRIDZIP_HZBIN_H
#define HYBRIDZIP_HZBIN_H

#include <functional>
#include <utility>
#include "hzrans.h"


class hzu_encoder {
private:
    std::function<uint64_t(void)> extract;
    hz_cross_encoder cross_encoder;
    hz_codec_callback callback;
    uint64_t *distptr;
    uint64_t size;
    int64_t index;
    hzrans64_t *state;

public:
    hzu_encoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_extractor(std::function<uint64_t(void)> _extract);

    void set_distribution(uint64_t *ptr);

    void set_callback(hz_codec_callback _callback);

    void set_cross_encoder(hz_cross_encoder _cross_encoder);

    void normalize(bool bypass_normalization=false);

    u32ptr encode();

    ~hzu_encoder();

};

class hzu_decoder {
private:
    hz_codec_callback callback;
    hz_cross_encoder cross_encoder;
    std::function<uint64_t()> symbol_callback;
    uint64_t *distptr;
    uint64_t size;
    hzrans64_t *state;

public:
    hzu_decoder(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void set_callback(hz_codec_callback _callback);

    void set_symbol_callback(std::function<uint64_t()> _symbol_callback);

    void set_cross_encoder(hz_cross_encoder _cross_encoder);

    u64ptr decode(uint32_t *raw, bool bypass_normalization=false);

    ~hzu_decoder();

};

#endif
