#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <hzip/other/platform.h>

struct hzrblob_t {
    uint64_t *data;
    uint64_t size;
    uint64_t o_size;
};

struct hzrblob_set{
    hzrblob_t *blobs;
    uint64_t count;
};


#endif
