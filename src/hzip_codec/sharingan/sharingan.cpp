#include "sharingan.h"
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/preprocessor/png_bundle.h>
#include <hzip_core/kernel/hzrans/hzrans64_codec.h>
#include <hzip_core/utils/distribution.h>
#include <hzip_codec/errors/codec.h>
#include <hzip_core/models/paeth.h>
#include <hzip_core/models/first_order_context.h>
#include <hzip_core/utils/utils.h>
#include <bitio/bitio.h>

bool hzcodec::Sharingan::validate_model(const rainman::ptr<uint8_t> &raw) {
    return raw.size() == sizeof(uint64_t) * 65536 * 3;
}

void hzcodec::Sharingan::load_models(
        rainman::ptr<hzmodels::FirstOrderContextModel> &models,
        const rainman::ptr<uint8_t> &raw
) {
    // The mstate model only stores the FOCM for the first 8-bits of a pixel per channel.
    auto stream = bitio::stream(raw.pointer(), raw.size());

    models = rainman::ptr<hzmodels::FirstOrderContextModel>(3);

    for (int i = 0; i < 3; i++) {
        models[i].set_alphabet_size(256);
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                models[i].set(j, k, stream.read(0x40));
            }
        }
    }
}

void hzcodec::Sharingan::init_models(rainman::ptr<hzmodels::FirstOrderContextModel> &models) {
    models = rainman::ptr<hzmodels::FirstOrderContextModel>(3);
    for (int i = 0; i < 3; i++) {
        models[i].set_alphabet_size(256);
    }
}

void hzcodec::Sharingan::train_models(
        rainman::ptr<hzmodels::FirstOrderContextModel> &models,
        uint64_t width,
        uint64_t height,
        uint8_t depth,
        uint8_t nchannels,
        const rainman::ptr<uint16_t> &buffer
) {
    uint64_t channel_offset_1 = width * height;
    uint8_t shift = depth >= 8 ? depth - 8 : 0;

    for (uint8_t channel = 0; channel < nchannels; channel++) {
        uint64_t offset = channel_offset_1 * channel;
        for (uint64_t i = 0; i < channel_offset_1; i++) {
            models[channel].update((buffer[offset + i] >> shift) & 0xff, 32);
        }
    }
}

rainman::ptr<uint64_t> hzcodec::Sharingan::freq_dist_residues(
        uint8_t channel_index,
        uint64_t per_channel_length,
        const rainman::ptr<uint16_t> &buffer
) {
    uint64_t channel_offset = per_channel_length * channel_index;
    auto output = rainman::ptr<uint64_t>(256);

    for (uint16_t i = 0; i < 256; i++) {
        output[i] = 1;
    }

    for (uint64_t i = 0; i < per_channel_length; i++) {
        output[buffer[channel_offset + i] & 0xff] += 32;
    }

    return output;
}

void hzcodec::Sharingan::apply_data_transforms(const PNGBundle &bundle, const rainman::ptr<uint16_t> &output) {
    auto color_type = bundle.ihdr.color_type;
    uint64_t per_channel_length = bundle.width * bundle.height;

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA) {
        for (int i = 0; i < bundle.nchannels; i++) {
            auto channel_offset = i * per_channel_length;
            auto paeth = hzmodels::PaethModel(bundle.buf, bundle.width, bundle.height, i);
            for (uint64_t y = 0; y < bundle.height; y++) {
                auto y_offset = y * bundle.width;
                for (uint64_t x = 0; x < bundle.width; x++) {
                    uint64_t index = channel_offset + y_offset + x;
                    uint16_t pred = paeth.predict(x, y);
                    uint16_t val = bundle.buf[index];
                    uint16_t diff = val | pred;
                    output[index] = diff;
                }
            }
        }
    } else if (color_type == PNG_COLOR_TYPE_PALETTE) {
        for (int i = 0; i < bundle.nchannels; i++) {
            auto channel_offset = i * per_channel_length;
            for (uint64_t y = 0; y < bundle.height; y++) {
                auto y_offset = y * bundle.width;
                for (uint64_t x = 0; x < bundle.width; x++) {
                    uint64_t index = channel_offset + y_offset + x;
                    output[index] = bundle.buf[index];
                }
            }
        }
    } else {
        throw CodecErrors::InvalidOperationException("[SHARINGAN] Unsupported color type");
    }
}

rainman::ptr<uint64_t> hzcodec::Sharingan::freq_dist_leading(
        uint8_t depth,
        uint8_t channel_index,
        uint64_t per_channel_length,
        const rainman::ptr<uint16_t> &buffer
) {
    uint64_t channel_offset = per_channel_length * channel_index;
    auto output = rainman::ptr<uint64_t>(256);

    for (uint16_t i = 0; i < 256; i++) {
        output[i] = 1;
    }

    uint8_t shift = depth >= 8 ? depth - 8 : 0;

    for (uint64_t i = 0; i < per_channel_length; i++) {
        output[(buffer[channel_offset + i] >> shift) & 0xff] += 32;
    }

    return output;
}

rainman::ptr<hzrans64_encoder_output> hzcodec::Sharingan::leading_encode(
        const PNGBundle &bundle,
        const rainman::ptr<hzmodels::FirstOrderContextModel> &models,
        const rainman::ptr<uint16_t> &data
) {
    auto per_channel_length = bundle.width * bundle.height;
    auto depth = bundle.depth;
    auto leading_outputs = rainman::ptr<hzrans64_encoder_output>(bundle.nchannels);
    auto color_type = bundle.ihdr.color_type;

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA || color_type == PNG_COLOR_TYPE_PALETTE) {
        for (uint64_t i = 0; i < bundle.nchannels; i++) {
            uint64_t index = per_channel_length * (i + 1);
            auto focm = models[i];

            auto cross_encoder = [&index, &focm, &data, depth](
                    const rainman::ptr<hzrans64_t> &state,
                    const rainman::ptr<HZ_Stack<uint32_t>> &_data
            ) {
                index--;
                uint8_t shift = depth >= 8 ? depth - 8 : 0;
                uint16_t curr_symbol = (data[index] >> shift) & 0xff;
                if (index == 0) {
                    state->ls = 65536;
                    state->bs = (uint64_t) curr_symbol << 16;
                } else {
                    uint16_t prev_symbol = (data[index - 1] >> shift) & 0xff;
                    auto dist = focm.get_dist(prev_symbol);

                    // Dynamic-normalization (Costly operation)
                    // Encode symbol

                    hzrans64_create_ftable_nf(state.pointer(), dist.pointer());
                    hzrans64_add_to_seq(state.pointer(), curr_symbol);

                    // revert model to previous state.
                    focm.revert(prev_symbol, curr_symbol, 32);
                }
            };

            auto encoder = hzrans64_encoder();

            encoder.set_header(256, 24, per_channel_length);
            encoder.set_distribution(hzip_get_init_dist(256));
            encoder.set_cross_encoder(cross_encoder);
            encoder.set_size(per_channel_length);

            leading_outputs[i] = encoder.encode();
        }
    } else {
        throw CodecErrors::InvalidOperationException("[SHARINGAN] Unsupported color type");
    }

    return leading_outputs;
}

rainman::ptr<hzrans64_encoder_output> hzcodec::Sharingan::residual_encode(
        const PNGBundle &bundle,
        const rainman::ptr<uint16_t> &data
) {
    auto per_channel_length = bundle.width * bundle.height;
    auto residual_outputs = rainman::ptr<hzrans64_encoder_output>(bundle.nchannels);

    for (uint64_t i = 0; i < bundle.nchannels; i++) {
        uint64_t index = per_channel_length * (i + 1);
        auto freq = freq_dist_residues(i, per_channel_length, data);

        auto cross_encoder = [&index, &freq, &data](
                const rainman::ptr<hzrans64_t> &state,
                const rainman::ptr<HZ_Stack<uint32_t>> &_data
        ) {
            index--;
            uint8_t curr_symbol = data[index] & 0xff;
            if (index == 0) {
                state->ls = 65536;
                state->bs = (uint64_t) curr_symbol << 16;
            } else {
                // Dynamic-normalization (Costly operation)
                // Encode symbol

                hzrans64_create_ftable_nf(state.pointer(), freq.pointer());
                hzrans64_add_to_seq(state.pointer(), curr_symbol);

                // revert model to previous state.
                freq[curr_symbol] -= 32;
            }
        };

        auto encoder = hzrans64_encoder();

        encoder.set_header(256, 24, per_channel_length);
        encoder.set_distribution(hzip_get_init_dist(256));
        encoder.set_cross_encoder(cross_encoder);
        encoder.set_size(per_channel_length);

        residual_outputs[i] = encoder.encode();
    }

    return residual_outputs;
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
    rainman::ptr<hzmodels::FirstOrderContextModel> models;

    init_models(models);

    auto cblob = rainman::ptr<HZ_Blob>();

    // Decode PNG
    auto bundle_builder = PNGBundleBuilder(blob->data);
    auto bundle = bundle_builder.read_pixels();

    auto mstate = blob->mstate;
    if (mstate->is_empty()) {
        mstate->alg = hzcodec::algorithms::SHARINGAN;
        mstate->data = rainman::ptr<uint8_t>();
    } else if (bundle.ihdr.color_type == PNG_COLOR_TYPE_RGB || bundle.ihdr.color_type == PNG_COLOR_TYPE_RGBA) {
        if (validate_model(mstate->data)) {
            load_models(models, mstate->data);
        }
    }

    uint8_t depth = bundle.depth;

    if (bundle.nchannels >= 3) {
        // Apply YCOCG color space transform on RGB image.
        if (depth <= 16) {
            auto transformer = hztrans::LinearU16ColorTransformer(bundle.width, bundle.height);
            bundle.buf = transformer.rgb_to_ycocg(bundle.buf);
        } else {
            throw CodecErrors::InvalidOperationException("[SHARINGAN] Unsupported bit depth");
        }
    }

    auto transformed_data = rainman::ptr<uint16_t>(bundle.buf.size());

    // Populate transformed_data with error-values.
    //todo: OpenCL implementation
    apply_data_transforms(bundle, transformed_data);
    train_models(models, bundle.width, bundle.height, depth, bundle.nchannels, transformed_data);

    // Encode leading 8-bits per pixel for all channels.
    auto leading_outputs = leading_encode(bundle, models, transformed_data);
    rainman::ptr<hzrans64_encoder_output> residual_outputs;

    // If the depth is greater than 8 then encode the residues with a dynamic frequency distribution
    if (depth > 8) {
        residual_outputs = residual_encode(bundle, transformed_data);
    }

    cblob->header = HZ_BlobHeader();
    auto raw_header = rainman::ptr<uint8_t>(6 + (0x40 * bundle.nchannels));

    auto hstream = bitio::stream(raw_header.pointer(), raw_header.size());

    cblob->header.raw = raw_header;

    // Encode width and height.
    hstream.write(bundle.width, 0x10);
    hstream.write(bundle.height, 0x10);

    // Encode bit-depth and nchannels
    hstream.write(bundle.depth, 0x8);
    hstream.write(bundle.nchannels, 0x8);

    uint64_t total_size = 0;

    // Write per-channel encoding length. (leading-byte)
    for (auto i = 0; i < leading_outputs.size(); i++) {
        auto chan_size = leading_outputs[i].n;
        total_size += chan_size << 2;
        hstream.write(chan_size, 0x40);
    }

    // Write residue lengths if required.
    if (depth > 8) {
        for (auto i = 0; i < residual_outputs.size(); i++) {
            auto chan_size = residual_outputs[i].n;
            total_size += chan_size << 2;
            hstream.write(chan_size, 0x40);
        }
    }

    auto blob_data = rainman::ptr<uint8_t>(total_size);
    auto bstream = bitio::stream(blob_data.pointer(), blob_data.size());

    for (auto i = 0; i < leading_outputs.size(); i++) {
        auto output = leading_outputs[i];
        for (auto j = 0; j < output.n; j++) {
            bstream.write(output.data[j], 0x20);
        }
    }


    // Write residues if required.
    if (depth > 8) {
        for (auto i = 0; i < residual_outputs.size(); i++) {
            auto output = residual_outputs[i];
            for (auto j = 0; j < output.n; j++) {
                bstream.write(output.data[j], 0x20);
            }
        }
    }

    cblob->data = blob_data;
    cblob->mstate = mstate;
    cblob->o_size = blob->o_size;
    cblob->size = blob_data.size();

    return cblob;
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {

}

rainman::ptr<HZ_MState> hzcodec::Sharingan::train(const rainman::ptr<HZ_Blob> &blob) {

}
