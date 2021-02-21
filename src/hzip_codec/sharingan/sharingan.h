/*
 * Sharingan is a lossless PNG remuxer that leverages color spaces for compression.
 * However, it performs worse for synthetic images.
 * Scheme: Lossless color space transform + Paeth Filter + First order context
 *
 * Sharingan's CPU implementation requires an mstate to be injected to make sure the algorithm runs at a sane speed. An
 * error is thrown if no GPU and mstate is found.
 * However, it might reduce compression efficiency.
 * On the other hand, Sharingan's GPU implementation requires no mstate. 32x32 image patches are processed in every work
 * item. However, if a mstate is given, only the transforms will be executed on the GPU. The rest is handled by the CPU.
 */

#ifndef HZIP_CODEC_SHARINGAN_H
#define HZIP_CODEC_SHARINGAN_H

#include <rainman/rainman.h>
#include <hzip_codec/compressor_base.h>
#include <hzip_core/preprocessor/png_bundle.h>

namespace hzcodec {
    class Sharingan : public AbstractCodec {
    private:
        void preprocess_data(const PNGBundle &bundle);

        void apply_filter(const PNGBundle &bundle);

    public:
        Sharingan() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override;
    };
}
#endif
