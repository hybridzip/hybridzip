/*
 * Sharingan is a lossless/lossy PNG remuxer that leverages color spaces for compression.
 * However, it performs worse for synthetic images.
 * Scheme: Lossless/Lossy color space transform + Paeth Filter + First order context
 */

#ifndef HYBRIDZIP_SHARINGAN_H
#define HYBRIDZIP_SHARINGAN_H

#include <rainman/rainman.h>
#include <hzip_core/models/first_order_context.h>
#include <hzip_core/preprocessor/png_bundle.h>
#include <hzip_codec/compressor_base.h>
#include <hzip_core/kernel/hzrans/hzrans64_codec.h>

namespace hzcodec {
    class Sharingan : public AbstractCodec {
    private:
        void load_models(rainman::ptr<hzmodels::FirstOrderContextModel> &models, const rainman::ptr<uint8_t> &raw);

        bool validate_model(const rainman::ptr<uint8_t> &raw);

        void init_models(rainman::ptr<hzmodels::FirstOrderContextModel> &models);

        void train_models(
                rainman::ptr<hzmodels::FirstOrderContextModel> &models,
                uint64_t width,
                uint64_t height,
                uint8_t depth,
                uint8_t nchannels,
                const rainman::ptr<uint16_t> &buffer
        );

        rainman::ptr<uint64_t> freq_dist_leading(
                uint8_t depth,
                uint8_t channel_index,
                uint64_t per_channel_length,
                const rainman::ptr<uint16_t> &buffer
        );

        rainman::ptr<uint64_t> freq_dist_residues(
                uint8_t channel_index,
                uint64_t per_channel_length,
                const rainman::ptr<uint16_t> &buffer
        );

        void apply_data_transforms(const PNGBundle &bundle, const rainman::ptr<uint16_t> &output);

        rainman::ptr<hzrans64_encoder_output> leading_encode(
                const PNGBundle &bundle,
                const rainman::ptr<hzmodels::FirstOrderContextModel> &models,
                const rainman::ptr<uint16_t> &data
        );

        rainman::ptr<hzrans64_encoder_output> residual_encode(
                const PNGBundle &bundle,
                const rainman::ptr<uint16_t> &data
        );

    public:
        Sharingan() = default;

        rainman::ptr<HZ_Blob> compress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_Blob> decompress(const rainman::ptr<HZ_Blob> &blob) override;

        rainman::ptr<HZ_MState> train(const rainman::ptr<HZ_Blob> &blob) override;
    };
}
#endif
