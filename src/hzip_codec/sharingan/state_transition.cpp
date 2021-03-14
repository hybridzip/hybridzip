#include "state_transition.h"
#include <hzip_core/runtime.h>
#include <hzip_core/config.h>
#include <hzip_core/opencl/cl_helper.h>
#include <hzip_core/kernel/hzrans/hzrans64.h>
#include <iostream>

/*
 * StateTransitionPair sequence (Static Model):- Leading Bytes (chan-1, chan-2, ..., chan-n) Residues (...)
 *
 * StateTransitionPair sequence (Dynamic Model) :-
 *  - Leading bytes
 *      - chan-x
 *          - Chunk-1, Chunk-2, ..., Chunk-n
 *  - Residues
 *      - chan-x
 *          - Chunk-1, Chunk-2, ..., Chunk-n
 */

SharinganStateTransition::SharinganStateTransition(
        const PNGBundle &bundle,
        uint64_t chunk_width,
        uint64_t chunk_height,
        bool is_dynamic,
        uint8_t locality_context_order,
        uint64_t learning_rate
) : _data(bundle.buf), _width(bundle.ihdr.width), _height(bundle.ihdr.height), _nchannels(bundle.nchannels),
    _chunk_width(chunk_width), _chunk_height(chunk_height), _is_dynamic(is_dynamic),
    _locality_context_order(locality_context_order), _bit_depth(bundle.ihdr.bit_depth),
    _learning_rate(learning_rate) {

}

void SharinganStateTransition::preprocess_maps() {
    _cumulative_maps = rainman::make_ptr3d<uint64_t>(_nchannels, 256, 256);

    hzrans64_t state;
    state.size = 256;
    state.scale = 24;

    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        auto ls_chan = _locality_maps[channel_index];
        auto bs_chan = _cumulative_maps[channel_index];
        for (uint64_t symbol = 0; symbol < 256; symbol++) {
            auto ls_arr = ls_chan[symbol];
            auto bs_arr = bs_chan[symbol];

            state.ftable = ls_arr.pointer();
            hzrans64_create_ftable_nf(&state, state.ftable);

            uint64_t bs = 0;
            for (uint64_t next_symbol = 0; next_symbol < 256; next_symbol++) {
                bs_arr[next_symbol] = bs;
                bs += ls_arr[next_symbol];
            }
        }
    }
}

void SharinganStateTransition::preprocess_dists() {
    _cumulative_dists = rainman::ptr2d<uint64_t>(_nchannels, 256);
    hzrans64_t state;
    state.size = 256;
    state.scale = 24;

    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        auto ls_arr = _frequency_dists[channel_index];
        auto bs_arr = _cumulative_dists[channel_index];

        state.ftable = ls_arr.pointer();

        hzrans64_create_ftable_nf(&state, state.ftable);

        uint64_t bs = 0;
        for (uint64_t next_symbol = 0; next_symbol < 256; next_symbol++) {
            bs_arr[next_symbol] = bs;
            bs += ls_arr[next_symbol];
        }
    }
}

void SharinganStateTransition::inject_model(
        const rainman::ptr3d<uint64_t> &locality_maps,
        const rainman::ptr2d<uint64_t> &frequency_dists
) {
    _locality_maps = locality_maps;
    _frequency_dists = frequency_dists;

    preprocess_maps();
    preprocess_dists();
}

SSTPair SharinganStateTransition::uniform_pair(uint64_t symbol) {
    return SSTPair{.ls=65536, .bs=static_cast<uint32_t>((symbol << 16))};
}

std::pair<rainman::virtual_array<SSTPair>, std::mutex &>
SharinganStateTransition::static_precode() {
    auto[cache, mutex] = Runtime::get_cache();

    std::scoped_lock<std::mutex> lock(mutex);

    uint8_t shift = _bit_depth > 8 ? _bit_depth - 8 : 0;
    uint64_t total_size = _nchannels * _width * _height;
    if (shift != 0) {
        total_size <<= 1;
    }

    auto array = rainman::virtual_array<SSTPair>(
            cache,
            total_size
    );

    uint64_t per_channel_size = _width * _height;

    // Generate SST pairs for leading-bytes
    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        uint64_t channel_offset = per_channel_size * channel_index;
        auto ls_chan = _locality_maps[channel_index];
        auto bs_chan = _cumulative_maps[channel_index];

        for (uint64_t y = 0; y < _height; y++) {
            uint64_t y_offset = channel_offset + y * _width;

            for (uint64_t x = 0; x < _width; x++) {
                uint64_t index = y_offset + x;

                uint64_t curr_symbol = _data[index] >> shift;

                if (x > 0) {
                    auto left_symbol = _data[index - 1];
                    uint32_t ls = ls_chan[left_symbol][curr_symbol];
                    uint32_t bs = bs_chan[left_symbol][curr_symbol];

                    array[index] = SSTPair{.ls=ls, .bs=bs};
                } else if (y > 0) {
                    if (_locality_context_order > 1) {
                        auto up_symbol = _data[index - _width];
                        uint32_t ls = ls_chan[up_symbol][curr_symbol];
                        uint32_t bs = bs_chan[up_symbol][curr_symbol];

                        array[index] = SSTPair{.ls=ls, .bs=bs};
                    } else {
                        array[index] = uniform_pair(curr_symbol);
                    }
                } else {
                    array[index] = uniform_pair(curr_symbol);
                }
            }
        }
    }

    uint64_t residue_offset = _nchannels * per_channel_size;

    // Generate SST pairs for residues, if any.
    if (shift > 0) {
        for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
            auto ls_arr = _frequency_dists[channel_index];
            auto bs_arr = _cumulative_dists[channel_index];

            uint64_t channel_offset = residue_offset + per_channel_size * channel_index;
            for (uint64_t y = 0; y < _height; y++) {
                uint64_t y_offset = channel_offset + y * _width;

                for (uint64_t x = 0; x < _width; x++) {
                    uint64_t index = y_offset + x;
                    uint64_t curr_symbol = _data[index];

                    uint32_t ls = ls_arr[curr_symbol];
                    uint32_t bs = bs_arr[curr_symbol];

                    array[index] = SSTPair{.ls=ls, .bs=bs};
                }
            }
        }
    }

    return std::pair<rainman::virtual_array<SSTPair>, std::mutex &>(array, mutex);
}

std::pair<rainman::virtual_array<SSTPair>, std::mutex &>
SharinganStateTransition::cpu_dynamic_precode() {
    auto[cache, mutex] = Runtime::get_cache();

    std::scoped_lock<std::mutex> lock(mutex);

    uint8_t shift = _bit_depth > 8 ? _bit_depth - 8 : 0;
    uint64_t total_size = _nchannels * _width * _height;
    if (shift != 0) {
        total_size <<= 1;
    }

    auto array = rainman::virtual_array<SSTPair>(
            cache,
            total_size
    );

    hzrans64_t state;
    state.size = 256;
    state.scale = 24;

    uint64_t ls_arr[256] = {1};
    state.ftable = ls_arr;

    uint64_t x_residue = _width % _chunk_width;
    uint64_t x_chunks = (_width / _chunk_width) + (x_residue != 0);

    uint64_t y_residue = _height % _chunk_height;
    uint64_t y_chunks = (_height / _chunk_height) + (y_residue != 0);

    uint64_t per_channel_offset = _width * _height;
    uint64_t residue_segment_offset = per_channel_offset * _nchannels;

    // Perform line-scan on each chunk.
    // Encode leading symbols first for more performance.

    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        uint64_t channel_offset = channel_index * per_channel_offset;

        for (uint64_t y_chunk_index = 0; y_chunk_index < y_chunks; y_chunk_index++) {
            uint64_t y_chunk_partial_offset = y_chunk_index * _chunk_height;
            uint64_t y_chunk_offset = channel_offset + y_chunk_partial_offset * _width;
            uint64_t chunk_h = y_chunk_partial_offset + _chunk_height > _height ? y_residue : _chunk_height;

            for (uint64_t x_chunk_index = 0; x_chunk_index < x_chunks; x_chunk_index++) {
                uint64_t x_chunk_partial_offset = x_chunk_index * _chunk_width;
                uint64_t x_chunk_offset = y_chunk_offset + x_chunk_partial_offset;

                uint64_t chunk_w = x_chunk_partial_offset + _chunk_width > _width ? x_residue : _chunk_width;

                // Create maps.
                uint64_t f_map[256][256] = {0};

                for (uint64_t y = 0; y < chunk_h; y++) {
                    uint64_t y_offset = x_chunk_offset + y * _width;
                    for (uint64_t x = 0; x < chunk_w; x++) {
                        uint64_t index = y_offset + x;
                        uint64_t leading_symbol = (_data[index] >> shift) & 0xff;

                        if (x > 0) {
                            uint64_t left_symbol = (_data[index - 1] >> shift) & 0xff;

                            hzrans64_create_ftable_nf(&state, f_map[left_symbol]);
                            hzrans64_add_to_seq(&state, leading_symbol);

                            auto val = SSTPair{
                                    .ls=static_cast<uint32_t>(state.ls),
                                    .bs=static_cast<uint32_t>(state.bs)
                            };

                            array.set(val, index);

                            f_map[left_symbol][leading_symbol] += _learning_rate;

                            if (y > 0) {
                                if (_locality_context_order > 1) {
                                    uint64_t up_symbol = (_data[index - _width] >> shift) & 0xff;
                                    f_map[up_symbol][leading_symbol] += _learning_rate;
                                }

                                if (_locality_context_order > 2) {
                                    uint64_t diag_symbol = (_data[index - _width - 1] >> shift) & 0xff;
                                    f_map[diag_symbol][leading_symbol] += (_learning_rate >> 1);
                                }
                            }
                        } else if (y > 0) {
                            if (_locality_context_order > 1) {
                                uint64_t up_symbol = (_data[index - _width] >> shift) & 0xff;

                                hzrans64_create_ftable_nf(&state, f_map[up_symbol]);
                                hzrans64_add_to_seq(&state, leading_symbol);

                                auto val = SSTPair{
                                        .ls=static_cast<uint32_t>(state.ls),
                                        .bs=static_cast<uint32_t>(state.bs)
                                };

                                array.set(val, index);

                                f_map[up_symbol][leading_symbol] += _learning_rate;
                            } else {
                                array.set(uniform_pair(leading_symbol), index);
                            }
                        } else {
                            array.set(uniform_pair(leading_symbol), index);
                        }
                    }
                }
            }
        }
    }

    if (shift != 0) {
        for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
            uint64_t channel_offset = residue_segment_offset + channel_index * per_channel_offset;

            for (uint64_t y_chunk_index = 0; y_chunk_index < y_chunks; y_chunk_index++) {
                uint64_t y_chunk_partial_offset = y_chunk_index * _chunk_height;
                uint64_t y_chunk_offset = channel_offset + y_chunk_partial_offset;
                uint64_t chunk_h = y_chunk_partial_offset + _chunk_height > _height ? y_residue : _chunk_height;

                for (uint64_t x_chunk_index = 0; x_chunk_index < x_chunks; x_chunk_index++) {
                    uint64_t x_chunk_partial_offset = x_chunk_index * _chunk_width;
                    uint64_t x_chunk_offset = y_chunk_offset + x_chunk_partial_offset;

                    uint64_t chunk_w = x_chunk_partial_offset + _chunk_width > _width ? x_residue : _chunk_width;

                    // Create dists.
                    uint64_t f_dist[256] = {1};

                    // Encode residual symbols if any.
                    for (uint64_t y = 0; y < chunk_h; y++) {
                        uint64_t y_offset = x_chunk_offset + y * _width;
                        for (uint64_t x = 0; x < chunk_w; x++) {
                            uint64_t index = y_offset + x;

                            uint64_t residual_symbol = _data[index] & 0xff;

                            if (x == 0 && y == 0) {
                                array.set(uniform_pair(residual_symbol), index);
                            } else {
                                state.ftable = ls_arr;

                                hzrans64_create_ftable_nf(&state, f_dist);
                                hzrans64_add_to_seq(&state, residual_symbol);

                                array.set(SSTPair{
                                        .ls=static_cast<uint32_t>(state.ls),
                                        .bs=static_cast<uint32_t>(state.bs)
                                }, index);
                            }

                            f_dist[residual_symbol] += (_learning_rate << 1);
                        }
                    }
                }
            }
        }
    }

    return std::pair<rainman::virtual_array<SSTPair>, std::mutex &>(array, mutex);
}

#ifdef HZIP_ENABLE_OPENCL

void SharinganStateTransition::register_opencl_program() {
    hzopencl::ProgramProvider::register_program("sharingan_dynamic",

#include "sharingan_dynamic.cl"

    );
}

std::pair<rainman::virtual_array<SSTPair>, std::mutex &> SharinganStateTransition::opencl_dynamic_precode() {

}

#endif
