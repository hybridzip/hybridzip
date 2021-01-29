#include "sharingan.h"
#include <hzip_core/core/preprocessor/transforms.h>
#include <hzip_core/core/preprocessor/png_bundle.h>
#include <hzip_core/errors/compression.h>
#include <hzip_core/core/models/paeth.h>
#include <hzip_core/core/models/first_order_context.h>

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
//    auto mstate = blob->mstate;
//    if (mstate == nullptr) {
//        // Default: lossless color space - YCoCg
//        mstate = rxnew(HZ_MState);
//        mstate->alg = hzcodec::algorithms::SHARINGAN;
//        mstate->length = 1;
//        mstate->data = rmalloc(uint8_t, 1);
//        mstate->data[0] = hztrans::HZ_COLOR_SPACE::YCOCG;
//    }
//
//    auto cblob = rxnew(HZ_Blob);
//    auto color_space = static_cast<hztrans::HZ_COLOR_SPACE>(mstate->data[0]);
//
//    // Decode PNG
//    auto bundle_builder = ronew(PNGBundleBuilder, blob->data, blob->o_size);
//    auto bundle = bundle_builder.read_pixels();
//
//    if (bundle.nchannels < 3) {
//        throw CompressionErrors::InvalidOperationException("Unsupported channel count");
//    }
//
//    // Apply color space transforms on bundle.
//
//    switch (bundle.depth) {
//        case 8: {
//            auto transformer = hztrans::RGBColorTransformer<uint8_t>(color_space);
//            for (int y = 0; y < bundle.height; y++) {
//                for (int x = 0; x < bundle.width; x++) {
//                    auto pixel = hztrans::ColorTransformPixel<uint8_t>{
//                        .x=(uint8_t)bundle.buf[0][y][x],
//                        .y=(uint8_t)bundle.buf[1][y][x],
//                        .z=(uint8_t)bundle.buf[2][y][x]
//                    };
//
//                    pixel = transformer.transform(pixel);
//                    bundle.buf[0][y][x] = pixel.x;
//                    bundle.buf[1][y][x] = pixel.y;
//                    bundle.buf[2][y][x] = pixel.z;
//                }
//            }
//
//            break;
//        }
//        case 16: {
//            auto transformer = hztrans::RGBColorTransformer<uint16_t>(color_space);
//            for (int y = 0; y < bundle.height; y++) {
//                for (int x = 0; x < bundle.width; x++) {
//                    auto pixel = hztrans::ColorTransformPixel<uint16_t>{
//                            .x=bundle.buf[0][y][x],
//                            .y=bundle.buf[1][y][x],
//                            .z=bundle.buf[2][y][x]
//                    };
//
//                    pixel = transformer.transform(pixel);
//                    bundle.buf[0][y][x] = pixel.x;
//                    bundle.buf[1][y][x] = pixel.y;
//                    bundle.buf[2][y][x] = pixel.z;
//                }
//            }
//            break;
//        }
//        default: throw CompressionErrors::InvalidOperationException("Unsupported bit depth");
//    }
//
//
//    for (int i = 0; i < bundle.nchannels; i++) {
//        auto buf = bundle.buf[i];
//        auto paeth = hzmodels::PaethModel(buf, bundle.width, bundle.height);
//
//    }
//
//
//
//    return cblob;
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {

}

rainman::ptr<HZ_MState> hzcodec::Sharingan::train(const rainman::ptr<HZ_Blob> &blob) {

}
