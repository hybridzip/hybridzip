#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include "../../other/platform.h"

struct hzrblob_t {
    uint32_t *data;
    uint64_t size;
};

struct hzrblob_set{
    hzrblob_t *blobs;
    uint32_t count;
};



#endif
