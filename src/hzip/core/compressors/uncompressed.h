#ifndef HYBRIDZIP_UNCOMPRESSED_H
#define HYBRIDZIP_UNCOMPRESSED_H

#include "compressor_base.h"
#include <hzip/errors/compression.h>

namespace hzcodec {
    class Uncompressed : public AbstractCodec, public rainman::context {
    public:
        Uncompressed() = default;

        HZ_Blob *compress(HZ_Blob *blob) override {
            auto cblob = rxnew(HZ_Blob);
            auto mstate = blob->mstate;

            if (mstate == nullptr) {
                mstate = rxnew(HZ_MState);
            }

            cblob->mstate = mstate;
            cblob->size = blob->size;
            cblob->o_size = blob->o_size;

            cblob->data = rmalloc(uint8_t, blob->size);
            for (uint64_t i = 0; i < blob->size; i++) {
                cblob->data[i] = blob->data[i];
            }

            cblob->status = false;
            return cblob;
        }

        HZ_Blob *decompress(HZ_Blob *blob) override {
            auto dblob = rxnew(HZ_Blob);
            auto mstate = blob->mstate;

            dblob->mstate = mstate;
            dblob->size = blob->size;
            dblob->o_size = blob->o_size;

            dblob->data = rmalloc(uint8_t, blob->size);
            for (uint64_t i = 0; i < blob->size; i++) {
                dblob->data[i] = blob->data[i];
            }

            dblob->status = false;
            return dblob;
        }

        HZ_MState *train(HZ_Blob *blob) override {
            throw CompressionErrors::InvalidOperationException("Cannot train models with algorithm UNCOMPRESSED");
        }

    };
}

#endif
