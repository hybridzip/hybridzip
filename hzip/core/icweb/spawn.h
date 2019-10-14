#ifndef HYBRIDZIP_INSANE_CONTEXT_WEB_SPAWN_H
#define HYBRIDZIP_INSANE_CONTEXT_WEB_SPAWN_H

#include <vector>
#include "../../other/platform.h"
#include "../entropy-coders/fgk.h"

namespace icweb {

    class spawn {
    private:
        HZIP_UINT ref_freq;
        HZIP_SIZE_T total_freq;
        HZIP_SIZE_T* freq;
        HZIP_UINT alphabet_size;
        float* noise_buf;
        hfc::fgk_tree tree;
    public:
        HZIP_UINT lru_index;

        spawn(HZIP_UINT, float*);
        ~spawn();
        void update(HZIP_UINT symbol);
        bin_t encode(HZIP_UINT symbol);
        void merge(spawn* s);
    };

}

#endif
