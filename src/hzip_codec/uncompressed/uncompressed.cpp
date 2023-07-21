#include "uncompressed.h"

rainman::ptr<HZ_Blob> hzcodec::Uncompressed::compress(const rainman::ptr<HZ_Blob> &blob) {
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

rainman::ptr<HZ_Blob> hzcodec::Uncompressed::decompress(const rainman::ptr<HZ_Blob> &blob) {
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

rainman::ptr<HZ_MState> hzcodec::Uncompressed::train(const rainman::ptr<HZ_Blob> &blob) {
    throw CodecErrors::InvalidOperationException("Cannot train models with algorithm: UNCOMPRESSED");
}
