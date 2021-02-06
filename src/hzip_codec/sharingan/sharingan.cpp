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

rainman::ptr<HZ_Blob> hzcodec::LosslessSharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
    auto mstate = blob->mstate;
    if (mstate->is_empty()) {
        // Default: lossless color space - YCoCg
        mstate->alg = hzcodec::algorithms::LOSSLESS_SHARINGAN;
        mstate->data = rainman::ptr<uint8_t>();
    }

    auto cblob = rainman::ptr<HZ_Blob>();
    auto color_space = hztrans::HZ_COLOR_SPACE::YCOCG;

    // Decode PNG
    auto bundle_builder = PNGBundleBuilder(blob->data.pointer(), blob->o_size);
    auto bundle = bundle_builder.read_pixels();

    if (bundle.nchannels < 3) {
        throw CodecErrors::InvalidOperationException("Unsupported channel count");
    }

    // Apply color space transforms on bundle.
    switch (bundle.depth) {
        case 8: {
            auto transformer = hztrans::RGBColorTransformer<uint8_t>(color_space);
            for (int y = 0; y < bundle.height; y++) {
                for (int x = 0; x < bundle.width; x++) {
                    auto pixel = hztrans::ColorTransformPixel<uint8_t>{
                            .x=(uint8_t) bundle.buf[0][y][x],
                            .y=(uint8_t) bundle.buf[1][y][x],
                            .z=(uint8_t) bundle.buf[2][y][x]
                    };

                    pixel = transformer.transform(pixel);
                    bundle.buf[0][y][x] = pixel.x;
                    bundle.buf[1][y][x] = pixel.y;
                    bundle.buf[2][y][x] = pixel.z;
                }
            }

            break;
        }
        case 16: {
            auto transformer = hztrans::RGBColorTransformer<uint16_t>(color_space);
            for (int y = 0; y < bundle.height; y++) {
                for (int x = 0; x < bundle.width; x++) {
                    auto pixel = hztrans::ColorTransformPixel<uint16_t>{
                            .x=bundle.buf[0][y][x],
                            .y=bundle.buf[1][y][x],
                            .z=bundle.buf[2][y][x]
                    };

                    pixel = transformer.transform(pixel);
                    bundle.buf[0][y][x] = pixel.x;
                    bundle.buf[1][y][x] = pixel.y;
                    bundle.buf[2][y][x] = pixel.z;
                }
            }
            break;
        }
        default:
            throw CodecErrors::InvalidOperationException("Unsupported bit depth");
    }

    auto outputs = rainman::ptr<hzrans64_encoder_output>(bundle.nchannels);
    uint64_t per_channel_length = bundle.width * bundle.height;

    for (int i = 0; i < bundle.nchannels; i++) {
        auto buf = bundle.buf[i];
        auto paeth = hzmodels::PaethModel(buf, bundle.width, bundle.height);
        auto focm = hzmodels::FirstOrderContextModel();
        uint8_t depth = bundle.depth;
        focm.set_alphabet_size(1ull << depth);

        auto zigzag_path = hztrans::ZigZagTransformer(bundle.width, bundle.height);
        auto zigzag_buf = rainman::ptr<uint16_t>(per_channel_length);

        bool init_flag = false;
        uint16_t prev_symbol = 0;
        auto index = 0;

        // Generate First-order context model on prediction-error
        for (auto coord : zigzag_path) {
            auto x = coord.x;
            auto y = coord.y;

            uint16_t pred = paeth.predict(x, y);
            uint16_t val = buf[y][x];

            uint16_t diff = val | pred;

            if (init_flag) {
                focm.update(prev_symbol, diff, 0x20);
            }

            init_flag = true;
            prev_symbol = diff;

            zigzag_buf[index++] = diff;
        }

        hz_assert(index == per_channel_length, "Failed to scan all symbols");

        auto cross_encoder = [&index, &focm, &zigzag_buf, depth](
                const rainman::ptr<hzrans64_t> &state,
                const rainman::ptr<HZ_Stack<uint32_t>> &_data
        ) {
            index--;
            uint16_t curr_symbol = zigzag_buf[index];
            if (index == 0) {
                state->ls = 65536;
                state->bs = (uint64_t) curr_symbol << (24 - depth);
            } else {
                uint16_t prev_symbol = zigzag_buf[index - 1];
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

        encoder.set_header(0x100, 24, per_channel_length);
        encoder.set_distribution(hzip_get_init_dist(0x100));
        encoder.set_cross_encoder(cross_encoder);
        encoder.set_size(per_channel_length);

        outputs[i] = encoder.encode();
    }

    cblob->header = HZ_BlobHeader();
    auto raw_header = rainman::ptr<uint8_t>(28);

    auto hstream = bitio::stream(raw_header.pointer(), raw_header.size());

    cblob->header.raw = raw_header;

    // Encode width and height.
    hstream.write(bundle.width, 0x10);
    hstream.write(bundle.height, 0x10);

    uint64_t total_size = 0;
    // Encode per-channel encoding length.
    for (auto i = 0; i < outputs.size(); i++) {
        auto chan_size = outputs[i].n;
        total_size += chan_size << 2;
        hstream.write(chan_size, 0x40);
    }

    auto blob_data = rainman::ptr<uint8_t>(total_size);
    auto bstream = bitio::stream(blob_data.pointer(), blob_data.size());

    for (auto i = 0; i < outputs.size(); i++) {
        auto output = outputs[i];
        for (auto j = 0; j < output.n; j++) {
            bstream.write(output.data[j], 0x20);
        }
    }

    cblob->data = blob_data;
    cblob->mstate = mstate;
    cblob->o_size = blob->o_size;
    cblob->size = blob_data.size();

    return cblob;
}

rainman::ptr<HZ_Blob> hzcodec::LosslessSharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {

}

rainman::ptr<HZ_MState> hzcodec::LosslessSharingan::train(const rainman::ptr<HZ_Blob> &blob) {

}
