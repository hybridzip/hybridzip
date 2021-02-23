#ifndef HZIP_SHARINGAN_STATE_TRANSITION_H
#define HZIP_SHARINGAN_STATE_TRANSITION_H

#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_WIDTH 0x40
#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_HEIGHT 0x40
#define HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LOCALITY_CONTEXT_ORDER 1

#include <rainman/rainman.h>
#include <hzip_core/preprocessor/png_bundle.h>

struct SharinganStateTransitionPair {
    uint32_t ls;
    uint32_t bs;
};

class SharinganStateTransition {
private:
    bool _is_dynamic;
    uint8_t _locality_context_order;

    uint64_t _chunk_width;
    uint64_t _chunk_height;

    rainman::ptr<uint16_t> _data;
    uint64_t _width;
    uint64_t _height;
    uint64_t _nchannels;
    uint8_t _bit_depth;

    rainman::ptr3d<uint64_t> _locality_maps;
    rainman::ptr3d<uint64_t> _cumulative_maps;
    rainman::ptr2d<uint64_t> _frequency_dists;
    rainman::ptr2d<uint64_t> _cumulative_dists;

    static SharinganStateTransitionPair uniform_pair(uint64_t symbol);

    void preprocess_maps();

    void preprocess_dists();

public:

    explicit SharinganStateTransition(
            const PNGBundle &bundle,
            uint64_t chunk_width = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_WIDTH,
            uint64_t chunk_height = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_CHUNK_HEIGHT,
            bool is_dynamic = true,
            uint8_t locality_context_order = HZIP_SHARINGAN_STATE_TRANSITION_DEFAULT_LOCALITY_CONTEXT_ORDER
    );

    void inject_model(const rainman::ptr3d<uint64_t> &locality_maps, const rainman::ptr2d<uint64_t> &frequency_dists);

    rainman::virtual_array<SharinganStateTransitionPair> static_precode();
};

#endif
