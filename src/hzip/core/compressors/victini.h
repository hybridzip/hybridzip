#ifndef HYBRIDZIP_VICTINI_H
#define HYBRIDZIP_VICTINI_H

#include <bitio/bitio.h>
#include <rainman/rainman.h>
#include <hzip/core/entropy/hzrans/hzbin.h>
#include <hzip/utils/distribution.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/core/models/models.h>

#include "compressor_base.h"

namespace hzcodec {
    class victini : public abstract_codec, public rainman::context {
    private:
        void
        gen_model_from_mstate(hz_mstate *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data, uint64_t length);

    public:
        victini() = default;

        hzblob_t *compress(hzblob_t *blob) override;

        hzblob_t *decompress(hzblob_t *blob) override;

        hz_mstate *train(hzblob_t *blob) override;

    };
}

#endif
