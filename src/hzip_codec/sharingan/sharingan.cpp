#include "sharingan.h"
#include <hzip_core/utils/distribution.h>
#include <hzip_core/preprocessor/transforms.h>

void hzcodec::Sharingan::preprocess_data(const PNGBundle &bundle) {
    auto color_type = bundle.ihdr.color_type;

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA) {
        auto transformer = hztrans::LinearU16ColorTransformer(bundle.ihdr.width, bundle.ihdr.height);
        transformer.rgb_to_ycocg(bundle.buf, true);
    }
}

void hzcodec::Sharingan::apply_filter(const PNGBundle &bundle) {

}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {

}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_Blob>();
}

rainman::ptr<HZ_MState> hzcodec::Sharingan::train(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_MState>();
}
