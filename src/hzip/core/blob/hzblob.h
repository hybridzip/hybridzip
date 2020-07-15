#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <malloc.h>
#include <hzip/utils/platform.h>
#include <hzip/core/compressors/algorithms.h>
#include <hzip/memory/mem_interface.h>

struct hzblob_t: public hz_mem_iface {
    uint32_t *data;
    uint8_t *o_data;
    uint64_t size;
    uint64_t o_size;
    hzcodec::ALGORITHM alg;

    hzblob_t() {
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
