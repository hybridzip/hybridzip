#ifndef HYBRIDZIP_HZBIN_H
#define HYBRIDZIP_HZBIN_H

#include <functional>
#include <utility>
#include <hzip/memory/mem_interface.h>
#include "hzrans.h"


class hzu_encoder: public hz_mem_iface {
private:
    hz_cross_codec cross_encoder;
    uint64_t *distptr{};
    uint64_t size{};
    int64_t index{};
    hzrans64_t *state{};

public:
    hzu_encoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void set_cross_encoder(hz_cross_codec _cross_encoder);

    void set_size(uint64_t _size);

    u32ptr encode();

    ~hzu_encoder();

};

class hzu_decoder: public hz_mem_iface {
private:
    hz_codec_callback callback;
    hz_cross_codec cross_decoder;
    uint64_t *sym_override_ptr;
    uint64_t *distptr;
    uint64_t size;
    hzrans64_t *state;

public:
    hzu_decoder() = default;

    void set_header(uint64_t alphabet_size, uint16_t scale, uint64_t buffer_size);

    void set_distribution(uint64_t *ptr);

    void override_symbol_ptr(uint64_t *ptr);

    void set_cross_decoder(hz_cross_codec _cross_decoder);

    u64ptr decode(uint32_t *raw);

    ~hzu_decoder();

};

#endif
