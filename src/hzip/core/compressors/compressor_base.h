#ifndef HYBRIDZIP_COMPRESSOR_BASE_H
#define HYBRIDZIP_COMPRESSOR_BASE_H

#include <hzip/core/blob/hzblob.h>

namespace hzcodec {
    class hz_abstract_codec {
    public:
        virtual hzblob_t *compress(hzblob_t *blob, hz_mstate *mstate) = 0;
        virtual hzblob_t *decompress(hzblob_t *blob, hz_mstate *mstate) = 0;
    };
}

#endif
