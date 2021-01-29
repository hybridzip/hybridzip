#ifndef HYBRIDZIP_HZRANS64_H
#define HYBRIDZIP_HZRANS64_H

#include <vector>
#include <chrono>
#include <functional>
#include <cstdint>
#include <cassert>
#include <stack>
#include <rainman/rainman.h>
#include <hzip_core/utils/platform.h>
#include <hzip_core/utils/stack.h>

struct hzrans64_t {
    uint64_t x{};
    uint64_t size{};
    uint64_t *ftable{};
    uint64_t scale{};
    const uint64_t lower_bound = 1ull << 31;
    uint64_t up_prefix{};
    uint64_t mask{};
    uint64_t ls{};
    uint64_t bs{};
    uint64_t count{};
};

uint64_t hzrans64_inv_bs(hzrans64_t *state, uint64_t bs);

void hzrans64_codec_init(hzrans64_t *state, uint64_t size, uint64_t scale);

void hzrans64_encode_s(hzrans64_t *state, HZ_Stack<uint32_t> *data);

void hzrans64_create_ftable_nf(hzrans64_t *state, uint64_t *freq);

void hzrans64_add_to_seq(hzrans64_t *state, uint64_t symbol);

void hzrans64_enc_flush(hzrans64_t *state, HZ_Stack<uint32_t> *data);

void hzrans64_dec_load_state(hzrans64_t *state, rainman::ptr<uint32_t> &data);

void hzrans64_decode_s(hzrans64_t *state, rainman::ptr<uint32_t> &data);

void hzrans64_destroy(hzrans64_t *state);

#endif
