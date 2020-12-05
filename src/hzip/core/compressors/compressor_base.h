#ifndef HYBRIDZIP_COMPRESSOR_BASE_H
#define HYBRIDZIP_COMPRESSOR_BASE_H

#include <hzip/core/blob/blob.h>

namespace hzcodec {
    class AbstractCodec {
    public:
        virtual HZ_Blob *compress(HZ_Blob *blob) = 0;

        virtual HZ_Blob *decompress(HZ_Blob *blob) = 0;

        virtual HZ_MState *train(HZ_Blob *blob) = 0;
    };
}

#endif
