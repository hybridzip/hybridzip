#include "state_transition.h"
#include <hzip_core/runtime.h>
#include <hzip_core/config.h>
#include <hzip_core/kernel/hzrans/hzrans64.h>

/*
 * StateTransitionPair sequence (Static Model):- Leading Bytes (chan-1, chan-2, ..., chan-n) Residues (...)
 */

SharinganStateTransition::SharinganStateTransition(
        const PNGBundle &bundle,
        uint64_t chunk_width,
        uint64_t chunk_height,
        bool is_dynamic,
        uint8_t locality_context_order
) : _data(bundle.buf), _width(bundle.ihdr.width), _height(bundle.ihdr.height), _nchannels(bundle.nchannels),
    _chunk_width(chunk_width), _chunk_height(chunk_height), _is_dynamic(is_dynamic),
    _locality_context_order(locality_context_order), _bit_depth(bundle.ihdr.bit_depth) {

}

void SharinganStateTransition::preprocess_maps() {
    _cumulative_maps = rainman::make_ptr3d<uint64_t>(_nchannels, 256, 256);

    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        auto ls_chan = _locality_maps[channel_index];
        auto bs_chan = _cumulative_maps[channel_index];
        for (uint64_t symbol = 0; symbol < 256; symbol++) {
            auto ls_arr = ls_chan[symbol];
            auto bs_arr = bs_chan[symbol];

            hzrans64_t state;
            state.size = 256;
            state.scale = 24;
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

    for (uint64_t channel_index = 0; channel_index < _nchannels; channel_index++) {
        auto ls_arr = _frequency_dists[channel_index];
        auto bs_arr = _cumulative_dists[channel_index];

        hzrans64_t state;
        state.size = 256;
        state.scale = 24;
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

SharinganStateTransitionPair SharinganStateTransition::uniform_pair(uint64_t symbol) {
    return SharinganStateTransitionPair{.ls=65536, .bs=static_cast<uint32_t>((symbol << 16))};
}

rainman::virtual_array<SharinganStateTransitionPair> SharinganStateTransition::static_precode() {
    std::scoped_lock<std::mutex> lock(HZRuntime::rainman_cache_mutex);

    auto array = rainman::virtual_array<SharinganStateTransitionPair>(
            Config::cache,
            _nchannels * _width * _height
    );

    uint64_t per_channel_size = _width * _height;
    uint8_t shift = _bit_depth > 8 ? _bit_depth - 8 : 0;

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

                    array[index] = SharinganStateTransitionPair{.ls=ls, .bs=bs};

                    continue;
                }

                if (x == 0 && y > 0) {
                    if (_locality_context_order > 1) {
                        auto up_symbol = _data[index - _width];
                        uint32_t ls = ls_chan[up_symbol][curr_symbol];
                        uint32_t bs = bs_chan[up_symbol][curr_symbol];

                        array[index] = SharinganStateTransitionPair{.ls=ls, .bs=bs};
                    } else {
                        array[index] = uniform_pair(curr_symbol);
                    }

                    continue;
                }

                if (x == 0 && y == 0) {
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

                    array[index] = SharinganStateTransitionPair{.ls=ls, .bs=bs};
                }
            }
        }
    }

    return array;
}


