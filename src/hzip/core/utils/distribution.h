#ifndef HYBRIDZIP_DISTRIBUTION_H
#define HYBRIDZIP_DISTRIBUTION_H

#include <cstdint>
#include <hzip/other/platform.h>

HZIP_FORCED_INLINE int32_t *hzip_get_init_dist() {
    auto dist = new int32_t[0x100];
    for(int i = 0; i < 0x100; i++) {
        dist[i] = 1;
    }
    return dist;
}
#endif
