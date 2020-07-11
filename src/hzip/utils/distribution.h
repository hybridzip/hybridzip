#ifndef HYBRIDZIP_DISTRIBUTION_H
#define HYBRIDZIP_DISTRIBUTION_H

#include <cstdint>
#include <hzip/memory/memmgr.h>
#include "platform.h"

HZ_INLINE uint64_t *hzip_get_init_dist(hz_memmgr* mgr, uint64_t size) {
    auto dist = mgr->hz_malloc<uint64_t>(size);
    for(int i = 0; i < size; i++) {
        dist[i] = 1;
    }
    return dist;
}

#endif
