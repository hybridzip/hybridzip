#ifndef HYBRIDZIP_UNCOMPRESSED_H
#define HYBRIDZIP_UNCOMPRESSED_H

#include "compressor_base.h"
#include <hzip_core/errors/compression.h>

namespace hzcodec {
    class Uncompressed : public AbstractCodec {
    public:
        Uncompressed() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override {
            auto cblob = rainman::ptr<HZ_Blob>();
            auto mstate = blob->mstate;

            cblob->mstate = mstate;
            cblob->size = blob->size;
            cblob->o_size = blob->o_size;

            cblob->data = rainman::ptr<uint8_t>(blob->size);
            for (uint64_t i = 0; i < blob->size; i++) {
                cblob->data[i] = blob->data[i];
            }

            cblob->status = false;
            return cblob;
        }

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override {
            auto dblob = rainman::ptr<HZ_Blob>();
            auto mstate = blob->mstate;

            dblob->mstate = mstate;
            dblob->size = blob->size;
            dblob->o_size = blob->o_size;

            dblob->data = rainman::ptr<uint8_t>(blob->size);
            for (uint64_t i = 0; i < blob->size; i++) {
                dblob->data[i] = blob->data[i];
            }

            dblob->status = false;
            return dblob;
        }

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override {
            throw CompressionErrors::InvalidOperationException("Cannot train models with algorithm: UNCOMPRESSED");
        }

    };
}

#endif
