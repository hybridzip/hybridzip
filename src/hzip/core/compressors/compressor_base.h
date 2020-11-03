#ifndef HYBRIDZIP_COMPRESSOR_BASE_H
#define HYBRIDZIP_COMPRESSOR_BASE_H

#include <hzip/core/blob/hzblob.h>

namespace hzcodec {
    class abstract_codec {
    public:
        virtual hzblob_t *compress(hzblob_t *blob) = 0;
        virtual hzblob_t *decompress(hzblob_t *blob) = 0;
        virtual hz_mstate *train(hzblob_t *blob) = 0;
    };
}

#endif
