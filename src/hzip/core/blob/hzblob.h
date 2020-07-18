#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <malloc.h>
#include <hzip/utils/platform.h>
#include <hzip/core/compressors/algorithms.h>
#include <hzip/memory/mem_interface.h>
#include <hzip/utils/common.h>

struct hzblob_t: public hz_mem_iface {
    uint64_t id;
    uint64_t ptable_id;

    uint32_t *data;
    uint8_t *o_data;
    uint64_t size;
    uint64_t o_size;
    hzcodec::ALGORITHM alg;

    hzblob_t() {
        id = 0;
        ptable_id = 0;
        data = nullptr;
        o_data = nullptr;
        size = 0;
        o_size = 0;
        alg = hzcodec::ALGORITHM::UNDEFINED;
    }

    void destroy() {
        HZ_FREE(data);
        HZ_FREE(o_data);
    }
};

struct hz_mstate: public hz_mem_iface {
    uint64_t id;
    bin_t *bins;
    uint64_t length;

    hz_mstate() {
        id = 0;
        bins = nullptr;
        length = 0;
    }
};

// deprecated
struct hzrblob_set {
    hzblob_t *blobs;
    uint64_t count;


    void destroy() {
        for (int i = 0; i < count; i++) {
            blobs[i].destroy();
        }
        free(blobs);
    }
};


#endif
