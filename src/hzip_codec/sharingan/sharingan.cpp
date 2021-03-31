#include "sharingan.h"
#include "state_transition.h"
#include <cmath>
#include <iostream>
#include <hzip_core/utils/distribution.h>
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/models/paeth.h>
#include <hzip_core/kernel/hzrans/hzrans64_codec.h>

void hzcodec::Sharingan::preprocess_data(const PNGBundle &bundle) {
    auto color_type = bundle.ihdr.color_type;

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA) {
        auto transformer = hztrans::LinearU16ColorTransformer(bundle.ihdr.width, bundle.ihdr.height);
        transformer.rgb_to_ycocg(bundle.buf, true);
    }
}

void hzcodec::Sharingan::apply_filter(const PNGBundle &bundle) {
    auto color_type = bundle.ihdr.color_type;

    if (color_type != PNG_COLOR_TYPE_PALETTE) {
        hzmodels::LinearU16PaethDifferential::filter(
                bundle.buf,
                bundle.ihdr.width,
                bundle.ihdr.height,
                bundle.nchannels,
                true,
                bundle.ihdr.bit_depth
        );
    }
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
    // ignore mstate for now.
    auto builder = PNGBundleBuilder(blob->data);
    auto bundle = builder.read_bundle();

    preprocess_data(bundle);
    apply_filter(bundle);

    auto state_transition = SharinganStateTransition(bundle);
    auto[virt_array, cache_mutex] = state_transition.dynamic_precode();

    rainman::virtual_array<SSTPair> state_trans = virt_array;

    hzrans64_encoder encoder;

    uint64_t index = state_trans.size();

    std::scoped_lock<std::mutex> lock(cache_mutex);

    encoder.set_header(0x100, 24, state_trans.size());
    encoder.set_cross_encoder([&state_trans, &index](
            const rainman::ptr<hzrans64_t> &state,
            const rainman::ptr<HZ_Stack<uint32_t>> &data
    ) {
        index--;
        auto transition = state_trans[index];
        state->ls = transition.ls;
        state->bs = transition.bs;
    });

    encoder.set_size(state_trans.size());
    encoder.set_distribution(hzip_get_init_dist(0x100));

    auto output = encoder.encode();

    std::cout << "Actual size: " << blob->data.size() << " bytes" << std::endl;
    std::cout << "Compressed size: " << output.n * 4 << " bytes" << std::endl;

//    double pbits = 0.0;
//    for (uint64_t i = 0; i < virt_array.size(); i++) {
//        pbits += log2(16777216.0 / double(virt_array[i].ls));
//    }
//
//    std::cout << "Actual size: " << blob->data.size() << " bytes" << std::endl;
//    std::cout << "Compressed size: " << pbits / 8 << " bytes" << std::endl;
//    std::cout << "Compression ratio: " << double(blob->data.size() * 8) / pbits << std::endl;

    return rainman::ptr<HZ_Blob>();
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_Blob>();
}

rainman::ptr<HZ_MState> hzcodec::Sharingan::train(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_MState>();
}
