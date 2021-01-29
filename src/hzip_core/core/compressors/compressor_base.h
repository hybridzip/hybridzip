#ifndef HYBRIDZIP_COMPRESSOR_BASE_H
#define HYBRIDZIP_COMPRESSOR_BASE_H

#include <hzip_core/core/blob/blob.h>

namespace hzcodec {
    class AbstractCodec {
    public:
        virtual rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) = 0;

        virtual rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) = 0;

        virtual rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) = 0;
    };
}

#endif
