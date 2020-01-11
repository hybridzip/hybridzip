#ifndef HYBRIDZIP_DISTRIBUTION_H
#define HYBRIDZIP_DISTRIBUTION_H

#include <cstdint>
#include <hzip/other/platform.h>

HZIP_FORCED_INLINE uint64_t *hzip_get_init_dist(uint64_t size) {
    auto dist = new uint64_t[size];
    for(int i = 0; i < size; i++) {
        dist[i] = 1;
    }
    return dist;
}
#endif
