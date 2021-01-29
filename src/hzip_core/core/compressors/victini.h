#ifndef HYBRIDZIP_VICTINI_H
#define HYBRIDZIP_VICTINI_H

#include <bitio/bitio.h>
#include <rainman/rainman.h>
#include <hzip_core/core/kernel/hzrans/hzrans64_codec.h>
#include <hzip_core/utils/distribution.h>
#include <hzip_core/core/preprocessor/transforms.h>
#include <hzip_core/core/models/models.h>

#include "compressor_base.h"

namespace hzcodec {
    class Victini : public AbstractCodec {
    private:
        static void _model_transform(
                const rainman::ptr<HZ_MState> &mstate,
                const rainman::ptr2d<uint64_t> &dict,
                const rainman::ptr2d<uint64_t> &cdict,
                const rainman::ptr<int16_t> &data,
                uint64_t length,
                bool training_mode = false
        );

    public:
        Victini() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override;

    };
}

#endif
