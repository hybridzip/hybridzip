#ifndef HYBRIDZIP_COMPRESSOR_ENUMS_H
#define HYBRIDZIP_COMPRESSOR_ENUMS_H

namespace hzcodec::algorithms {
    enum ALGORITHM {
        UNCOMPRESSED = 0x0,
        VICTINI = 0x1,
        SHARINGAN = 0x2
    };
}

#endif
