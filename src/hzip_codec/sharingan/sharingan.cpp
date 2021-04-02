#include "sharingan.h"
#include <hzip_core/utils/distribution.h>
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/models/paeth.h>
#include <hzip_core/kernel/hzrans/hzrans64_codec.h>
#include <hzip_core/utils/utils.h>
#include "state_transition.h"

void hzcodec::Sharingan::preprocess_data(const PNGBundle &bundle) {
    auto color_type = bundle.ihdr.color_type;

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA) {
        auto transformer = hztrans::LinearU16XColorTransformer(
                bundle.ihdr.width,
                bundle.ihdr.height,
                bundle.ihdr.bit_depth
        );

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

HZ_BlobHeader hzcodec::Sharingan::generate_blob_header(const PNGBundle &bundle) {
    auto header = HZ_BlobHeader();

    std::vector<bin_t> data;

    // Store custom metadata
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.nchannels), .n=8});

    // Store IHDR
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.width), .n=32});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.height), .n=32});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.bit_depth), .n=8});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.color_type), .n=8});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.interlace_method), .n=8});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.filter_method), .n=8});
    data.push_back(bin_t{.obj=static_cast<uint64_t>(bundle.ihdr.compression_method), .n=8});

    // Store PLTE
    data.push_back(bin_t{.obj=bundle.plte.colors.size(), .n=32});
    for (int i = 0; i < bundle.plte.colors.size(); i++) {
        data.push_back(bin_t{.obj=bundle.plte.colors[i].red, .n=8});
        data.push_back(bin_t{.obj=bundle.plte.colors[i].green, .n=8});
        data.push_back(bin_t{.obj=bundle.plte.colors[i].blue, .n=8});
    }

    header.raw = hz_vecbin_to_raw(data);

    return header;
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::compress(const rainman::ptr<HZ_Blob> &blob) {
    auto mstate = rainman::ptr<HZ_MState>();
    mstate->alg = hzcodec::algorithms::SHARINGAN;

    auto builder = PNGBundleBuilder(blob->data);
    auto bundle = builder.read_bundle();

    preprocess_data(bundle);
    apply_filter(bundle);

    auto chunk_width = blob->codec_params.get<uint64_t>("chunk_width", "64");
    auto chunk_height = blob->codec_params.get<uint64_t>("chunk_height", "64");
    auto locality_context_order = blob->codec_params.get<uint64_t>("locality_context_order", "3");
    auto learning_rate = blob->codec_params.get<uint64_t>("learning_rate", "32");

    auto state_transition = SharinganStateTransition(
            bundle,
            chunk_width,
            chunk_height,
            locality_context_order,
            learning_rate
    );

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
    auto cblob = rainman::ptr<HZ_Blob>();

    auto header = generate_blob_header(bundle);

    cblob->data = hz_u32_to_u8ptr(output.data, output.n);
    cblob->size = output.n << 2;
    cblob->o_size = blob->data.size();
    cblob->mstate = mstate;
    cblob->header = header;

    return cblob;
}

rainman::ptr<HZ_Blob> hzcodec::Sharingan::decompress(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_Blob>();
}

rainman::ptr<HZ_MState> hzcodec::Sharingan::train(const rainman::ptr<HZ_Blob> &blob) {
    return rainman::ptr<HZ_MState>();
}
