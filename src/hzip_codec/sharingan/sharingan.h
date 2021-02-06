/*
 * LosslessSharingan is a lossless/lossy PNG remuxer that leverages color spaces for compression.
 * However, it performs worse for synthetic images.
 * Scheme: Lossless/Lossy color space transform + Paeth Filter + First order context
 */

#ifndef HYBRIDZIP_SHARINGAN_H
#define HYBRIDZIP_SHARINGAN_H

#include <rainman/rainman.h>
#include "hzip_codec/compressor_base.h"

namespace hzcodec {
    class LosslessSharingan : public AbstractCodec {
    public:
        LosslessSharingan() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override;
    };
}
#endif
