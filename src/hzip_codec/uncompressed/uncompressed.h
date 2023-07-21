#ifndef HYBRIDZIP_UNCOMPRESSED_H
#define HYBRIDZIP_UNCOMPRESSED_H

#include "hzip_codec/compressor_base.h"
#include <hzip_codec/errors/codec.h>

namespace hzcodec {
    class Uncompressed : public AbstractCodec {
    public:
        Uncompressed() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override;

    };
}

#endif
