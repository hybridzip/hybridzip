#ifndef HYBRIDZIP_HZRBIN_H
#define HYBRIDZIP_HZRBIN_H

#include <functional>
#include <hzip/bitio/bitio.h>
#include "hzrans.h"


class HZRUEncoder {
private:
    std::function<uint64_t(bitio::bitio_stream)> extract;

public:
    HZRUEncoder(uint64_t alphabet_size, uint16_t scale) {

    }
};

#endif
