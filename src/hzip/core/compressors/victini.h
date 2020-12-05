#ifndef HYBRIDZIP_VICTINI_H
#define HYBRIDZIP_VICTINI_H

#include <bitio/bitio.h>
#include <rainman/rainman.h>
#include <hzip/core/kernel/hzrans/hzrans64_codec.h>
#include <hzip/utils/distribution.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/core/models/models.h>

#include "compressor_base.h"

namespace hzcodec {
    class Victini : public AbstractCodec, public rainman::context {
    private:
        void
        gen_model_from_mstate(HZ_MState *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data, uint64_t length,
                              bool training_mode = false);

    public:
        Victini() = default;

        HZ_Blob *compress(HZ_Blob *blob) override;

        HZ_Blob *decompress(HZ_Blob *blob) override;

        HZ_MState *train(HZ_Blob *blob) override;

    };
}

#endif
