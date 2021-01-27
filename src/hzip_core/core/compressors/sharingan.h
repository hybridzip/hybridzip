/*
 * Sharingan is a lossless/lossy PNG remuxer that leverages color spaces for compression.
 * However, it performs worse for synthetic images.
 * Scheme: Lossless/Lossy color space transform + Paeth Filter + First order context
 */

#ifndef HYBRIDZIP_SHARINGAN_H
#define HYBRIDZIP_SHARINGAN_H

#include <rainman/rainman.h>
#include "compressor_base.h"

namespace hzcodec {
    class Sharingan : public AbstractCodec, public rainman::context {
    public:
        Sharingan() = default;

        HZ_Blob *compress(HZ_Blob *blob) override;

        HZ_Blob *decompress(HZ_Blob *blob) override;

        HZ_MState *train(HZ_Blob *blob) override;
    };
}
#endif
