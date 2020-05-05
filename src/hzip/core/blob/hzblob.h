#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <memory>
#include <hzip/utils/platform.h>

struct hzrblob_t {
    uint32_t *data;
    uint64_t size;
    uint64_t o_size;

    void destroy() {
        free(data);
    }
};

struct hzrblob_set {
    hzrblob_t *blobs;
    uint64_t count;


    void destroy() {
        for (int i = 0; i < count; i++) {
            blobs[i].destroy();
        }
        free(blobs);
    }
};


#endif
