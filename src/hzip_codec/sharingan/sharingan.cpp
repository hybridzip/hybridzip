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

void hzcodec::Sharingan::load_models(const rainman::ptr<uint8_t> &raw) {
    // The mstate model only stores the FOCM for the first 8-bits of a pixel per channel.
    auto stream = bitio::stream(raw.pointer(), raw.size());

    _models = rainman::ptr<hzmodels::FirstOrderContextModel>(3);

    for (int i = 0; i < 3; i++) {
        _models[i].set_alphabet_size(256);
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                _models[i].set(j, k, stream.read(0x40));
            }
        }
    }
}

void hzcodec::Sharingan::init_models() {
    _models = rainman::ptr<hzmodels::FirstOrderContextModel>(3);
    for (int i = 0; i < 3; i++) {
        _models[i].set_alphabet_size(256);
    }
}

void hzcodec::Sharingan::train_models(
        uint64_t width,
        uint64_t height,
        uint8_t depth,
        const rainman::ptr<uint16_t> &buffer
) {
    uint64_t channel_offset_1 = width * height;
    uint64_t channel_offset_2 = channel_offset_1 << 1;
    uint8_t shift = depth - 8;
    for (uint64_t i = 0; i < channel_offset_1; i++) {
        _models[0].update((buffer[i] >> shift) & 0xff, 32);
        _models[1].update((buffer[channel_offset_1 + i] >> shift) & 0xff, 32);
        _models[2].update((buffer[channel_offset_2 + i] >> shift) & 0xff, 32);
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

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
    init_models();

    auto mstate = blob->mstate;
    if (mstate->is_empty()) {
        mstate->alg = hzcodec::algorithms::SHARINGAN;
        mstate->data = rainman::ptr<uint8_t>();
    } else {
        if (validate_model(mstate->data)) {
            load_models(mstate->data);
        }
    }

    auto cblob = rainman::ptr<HZ_Blob>();

    // Decode PNG
    auto bundle_builder = PNGBundleBuilder(blob->data.pointer(), blob->o_size);
    auto bundle = bundle_builder.read_pixels();

    if (bundle.nchannels < 3) {
        throw CodecErrors::InvalidOperationException("Unsupported channel count");
    }

    uint8_t depth = bundle.depth;
    // Apply color space transforms on bundle.
    if (depth == 8 || depth == 16) {
        auto transformer = hztrans::LinearU16ColorTransformer(bundle.width, bundle.height);
        bundle.buf = transformer.rgb_to_ycocg(bundle.buf);
    } else {
        throw CodecErrors::InvalidOperationException("Unsupported bit depth");
    }

    auto leading_outputs = rainman::ptr<hzrans64_encoder_output>(bundle.nchannels);
    auto residual_outputs = rainman::ptr<hzrans64_encoder_output>(bundle.nchannels);

    uint64_t per_channel_length = bundle.width * bundle.height;
    auto encoded_buffer = rainman::ptr<uint16_t>(bundle.buf.size());

    // Populate encoded_buffer with error-values.
    //todo: OpenCL implementation
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
                encoded_buffer[index] = diff;
            }
        }
    }

    train_models(bundle.width, bundle.height, depth, encoded_buffer);

    // Encode leading 8-bits per pixel for all channels.
    for (uint64_t i = 0; i < bundle.nchannels; i++) {
        uint64_t index = per_channel_length * (i + 1);
        auto focm = _models[i];

        auto cross_encoder = [&index, &focm, &encoded_buffer, depth](
                const rainman::ptr<hzrans64_t> &state,
                const rainman::ptr<HZ_Stack<uint32_t>> &_data
        ) {
            index--;
            uint8_t shift = depth - 8;
            uint16_t curr_symbol = (encoded_buffer[index] >> shift) & 0xff;
            if (index == 0) {
                state->ls = 65536;
                state->bs = (uint64_t) curr_symbol << 16;
            } else {
                uint16_t prev_symbol = (encoded_buffer[index - 1] >> shift) & 0xff;
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

    // If the depth is greater than 8 then encode the residues with a dynamic frequency distribution
    if (depth > 8) {
        for (uint64_t i = 0; i < bundle.nchannels; i++) {
            uint64_t index = per_channel_length * (i + 1);
            auto freq = freq_dist_residues(i, per_channel_length, encoded_buffer);

            auto cross_encoder = [&index, &freq, &encoded_buffer](
                    const rainman::ptr<hzrans64_t> &state,
                    const rainman::ptr<HZ_Stack<uint32_t>> &_data
            ) {
                index--;
                uint8_t curr_symbol = encoded_buffer[index] & 0xff;
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
