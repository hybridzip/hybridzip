#ifndef HYBRIDZIP_VICTINI_H
#define HYBRIDZIP_VICTINI_H

#include <bitio/bitio.h>
#include <hzip/core/entropy/hzrans/hzbin.h>
#include <hzip/utils/distribution.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/core/models/models.h>
#include <hzip/memory/mem_interface.h>

#include "compressor_base.h"

namespace hzcodec {
    class victini : public hz_abstract_codec, public hz_mem_iface {
    private:
        void gen_model_from_mstate(hz_mstate *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data, uint64_t length,
                                   uint64_t bwt_index);

    public:
        victini() = default;

        hzblob_t *compress(hzblob_t *blob) override;

        hzblob_t *decompress(hzblob_t *blob) override;
    };
}

#endif