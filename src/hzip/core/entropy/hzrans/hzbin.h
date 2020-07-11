#ifndef HYBRIDZIP_HZBIN_H
#define HYBRIDZIP_HZBIN_H

#include <functional>
#include <utility>
#include <hzip/memory/mem_interface.h>
#include "hzrans.h"


class hzu_encoder: public hz_mem_iface {
private:
    std::function<uint64_t(void)> extract;
    hz_cross_encoder cross_encoder;
    hz_codec_callback callback;
    uint64_t *distptr{};
    uint64_t size{};
    int64_t index{};
    hzrans64_t *state{};

public:
    hzu_encoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_extractor(std::function<uint64_t(void)> _extract);

    void set_distribution(uint64_t *ptr);

    void set_callback(hz_codec_callback _callback);

    void set_cross_encoder(hz_cross_encoder _cross_encoder);

    void normalize(bool bypass_normalization=false);

    u32ptr encode();

    ~hzu_encoder();

};

class hzu_decoder: public hz_mem_iface {
private:
    hz_codec_callback callback;
    hz_cross_encoder cross_encoder;
    std::function<uint64_t()> symbol_callback;
    uint64_t *distptr;
    uint64_t size;
    hzrans64_t *state;

public:
    hzu_decoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void set_callback(hz_codec_callback _callback);

    void set_symbol_callback(std::function<uint64_t()> _symbol_callback);

    void set_cross_encoder(hz_cross_encoder _cross_encoder);

    u64ptr decode(uint32_t *raw, bool bypass_normalization=false);

    ~hzu_decoder();

};

#endif
