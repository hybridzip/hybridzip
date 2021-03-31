#ifndef HZIP_SHARINGAN_STATE_TRANSITION_H
#define HZIP_SHARINGAN_STATE_TRANSITION_H

#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_WIDTH 0x40
#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_HEIGHT 0x40
#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LOCALITY_CONTEXT_ORDER 3
#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LEARNING_RATE 0x20

#include <rainman/rainman.h>
#include <hzip_core/preprocessor/png_bundle.h>

struct SSTPair {
    uint32_t ls;
    uint32_t bs;
};

class SharinganStateTransition {
private:
    uint8_t _locality_context_order;

    uint64_t _chunk_width;
    uint64_t _chunk_height;

    rainman::ptr<uint16_t> _data;
    uint64_t _width;
    uint64_t _height;
    uint64_t _nchannels;
    uint8_t _bit_depth;
    uint64_t _learning_rate;

    rainman::ptr3d<uint64_t> _locality_maps;
    rainman::ptr3d<uint64_t> _cumulative_maps;
    rainman::ptr2d<uint64_t> _frequency_dists;
    rainman::ptr2d<uint64_t> _cumulative_dists;

    static SSTPair uniform_pair(uint64_t symbol);

    void preprocess_maps();

    void preprocess_dists();

public:

    explicit SharinganStateTransition(
            const PNGBundle &bundle,
            uint64_t chunk_width = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_WIDTH,
            uint64_t chunk_height = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_HEIGHT,
            bool is_dynamic = true,
            uint8_t locality_context_order = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LOCALITY_CONTEXT_ORDER,
            uint64_t learning_rate = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LEARNING_RATE
    );

    void inject_model(const rainman::ptr3d<uint64_t> &locality_maps, const rainman::ptr2d<uint64_t> &frequency_dists);

    std::pair<rainman::virtual_array<SSTPair>, std::mutex &> static_precode();

    std::pair<rainman::virtual_array<SSTPair>, std::mutex &> dynamic_precode();

    std::pair<rainman::virtual_array<SSTPair>, std::mutex &> cpu_dynamic_precode();

#ifdef HZIP_ENABLE_OPENCL

    static void register_opencl_program();

    std::pair<rainman::virtual_array<SSTPair>, std::mutex &> opencl_dynamic_precode();

#endif
};

#endif
