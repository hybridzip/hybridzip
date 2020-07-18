#ifndef HYBRIDZIP_HZRANS64_H
#define HYBRIDZIP_HZRANS64_H

#include <vector>
#include <chrono>
#include <functional>
#include <cstdint>
#include <cassert>
#include <stack>
#include <malloc.h>
#include <hzip/utils/platform.h>
#include <hzip/utils/stack.h>
#include <hzip/memory/mem_interface.h>

struct hzrans64_t: public hz_mem_iface {
    uint64_t x;
    uint64_t size;
    uint64_t *ftable;
    uint64_t scale;
    const uint64_t lower_bound = 1ull << 31;
    uint64_t up_prefix;
    uint64_t mask;
    uint64_t ls;
    uint64_t bs;
    uint64_t count;
};

 uint64_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs);

 void hzrans64_codec_init(hzrans64_t *state, uint64_t size, uint64_t scale);

 void hzrans64_encode_s(hzrans64_t *state, hz_stack<uint32_t> *data);

 void hzrans64_create_ftable_nf(hzrans64_t *state, uint64_t *freq);

 void
hzrans64_add_to_seq(hzrans64_t *state, uint64_t symbol);

 void hzrans64_enc_flush(hzrans64_t *state, hz_stack<uint32_t> *data);

 void hzrans64_dec_load_state(hzrans64_t *state, uint32_t **data);

 void
hzrans64_decode_s(hzrans64_t *state, uint64_t *_ls, uint32_t **data, uint64_t *sym,
                  bool bypass_normalization = false);
#endif
