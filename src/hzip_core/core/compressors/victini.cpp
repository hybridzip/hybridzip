#include "victini.h"
#include <hzip_core/utils/utils.h>
#include <hzip_core/utils/stack.h>
#include <hzip_core/errors/compression.h>

rainman::ptr<HZ_Blob> hzcodec::Victini::compress(const rainman::ptr<HZ_Blob> &blob) {
    auto mstate = blob->mstate;
    auto header = blob->header;
    auto length = blob->o_size;

    auto data = rainman::ptr<int16_t>(length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->data[i];
    }

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(data, 0x100);

    auto bwt_index = bwt.transform();

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100);
    mtf.transform();

    auto dict = rainman::make_ptr2d<uint64_t>(256, 256);
    auto cdict = rainman::make_ptr2d<uint64_t>(256, 256);

    // Write blob-header.
    header.raw = rainman::ptr<uint8_t>(8);

    hz_u64_to_u8buf(bwt_index, header.raw);

    // Manage mstate
    _model_transform(mstate, dict, cdict, data, length);

    // Perform cross-encoding.
    uint64_t index = length;

    auto cross_encoder = [dict, cdict, &index, data](
            const rainman::ptr<hzrans64_t> &state,
            const rainman::ptr<HZ_Stack<uint32_t>> &_data
    ) {
        index--;
        if (index != 0) {
            state->ls = dict[data[index - 1]][data[index]];
            state->bs = cdict[data[index - 1]][data[index]];
        } else {
            state->ls = 65536;
            state->bs = ((uint64_t) data[index]) << 16;
        }
    };

    auto encoder = hzrans64_encoder();

    encoder.set_header(0x100, 24, length);
    encoder.set_distribution(hzip_get_init_dist(0x100));
    encoder.set_cross_encoder(cross_encoder);
    encoder.set_size(length);

    hzrans64_encoder_output blob_data = encoder.encode();

    auto cblob = rainman::ptr<HZ_Blob>();

    cblob->data = u32_to_u8ptr(blob_data.data, blob_data.n);
    cblob->size = blob_data.n << 2;
    cblob->o_size = length;
    cblob->mstate = mstate;
    cblob->mstate->alg = hzcodec::algorithms::VICTINI;
    cblob->header = header;

    return cblob;
}

rainman::ptr<HZ_Blob> hzcodec::Victini::decompress(const rainman::ptr<HZ_Blob> &blob) {
    auto mstate = blob->mstate;
    uint64_t length = blob->o_size;

    // Parse blob_header
    uint64_t bwt_index = hz_u8buf_to_u64(blob->header.raw);

    auto m_stream = bitio::stream(mstate->data.pointer(), mstate->size());

    auto dict = rainman::make_ptr2d<uint64_t>(0x100, 0x100);
    auto cdict = rainman::make_ptr2d<uint64_t>(0x100, 0x100);

    // populate the dictionary.
    for (int i = 0; i < 0x100; i++) {
        for (int j = 0; j < 0x100; j++) {
            dict[i][j] = m_stream.read(0x40);
        }
    }

    // Normalize dictionary.
    _normalize(dict, cdict);

    // create a cross-decoder.
    // a cross-decoder usually handles add_to_seq for the core entropy codec.
    int prev_symbol = -1;
    auto sym_optr = rainman::ptr<uint64_t>();

    auto cross_decoder = [&dict, &cdict, &prev_symbol, &sym_optr](
            const rainman::ptr<hzrans64_t> &state,
            const rainman::ptr<HZ_Stack<uint32_t>> &data
    ) {
        uint64_t x = state->x;
        uint64_t bs = x & state->mask;
        uint8_t symbol = 0xff;

        if (prev_symbol == -1) {
            for (int i = 0; i < 0x100; i++) {
                if (((i + 1) << 16) > bs) {
                    symbol = i;
                    break;
                }
            }
            state->ls = 65536;
            state->bs = symbol << 16;
        } else {
            for (int i = 0; i < 0x100; i++) {
                if (cdict[prev_symbol][i] > bs) {
                    symbol = i - 1;
                    break;
                }
            }
            state->ls = dict[prev_symbol][symbol];
            state->bs = cdict[prev_symbol][symbol];
        }

        prev_symbol = symbol;
        *sym_optr = prev_symbol;
    };

    auto decoder = hzrans64_decoder();

    decoder.set_header(0x100, 24, length);
    decoder.set_cross_decoder(cross_decoder);
    decoder.set_distribution(hzip_get_init_dist(0x100));
    decoder.override_symbol_ptr(sym_optr);

    rainman::ptr<uint32_t> blob_data = u8_to_u32ptr(blob->data, blob->size);

    hzrans64_decoder_output dataptr = decoder.decode(blob_data);

    auto sdata = rainman::ptr<int16_t>(length);

    for (uint64_t i = 0; i < length; i++) {
        sdata[i] = dataptr.data[i];
    }

    auto mtf = hztrans::MoveToFrontTransformer(sdata, 0x100);
    mtf.invert();

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(sdata, 0x100);

    bwt.invert(bwt_index);

    auto dblob = rainman::ptr<HZ_Blob>();

    dblob->data = rainman::ptr<uint8_t>(length);
    dblob->o_size = length;
    dblob->mstate = mstate;
    dblob->mstate->alg = hzcodec::algorithms::VICTINI;

    for (uint64_t i = 0; i < length; i++) {
        dblob->data[i] = sdata[i];
    }

    return dblob;
}

void hzcodec::Victini::_model_transform(
        const rainman::ptr<HZ_MState> &mstate,
        const rainman::ptr2d<uint64_t> &dict,
        const rainman::ptr2d<uint64_t> &cdict,
        const rainman::ptr<int16_t> &data,
        uint64_t length,
        bool training_mode
) {
    if (mstate->is_empty()) {
        auto focm = hzmodels::FirstOrderContextModel();

        focm.set_alphabet_size(0x100);
        // Now we contruct a First-Order-Context-Dictionary.
        for (uint64_t i = 0; i < length; i++) {
            focm.update(data[i], 32);
        }

        // Now we perform a one-time normalization for all possible contexts to increase speed.
        // Populate p-values and cumulative values.

        // Generate mstate object (without normalization).
        mstate->data = rainman::ptr<uint8_t>(65537 << 3);
        auto m_stream = bitio::stream(mstate->data.pointer(), mstate->data.size());

        for (int i = 0; i < 0x100; i++) {
            auto row = focm.get_dist(i);
            auto target = dict[i];
            for (int j = 0; j < 0x100; j++) {
                auto v = row[j];
                target[j] = v;
                m_stream.write(v, 0x40);
            }
        }

        _normalize(dict, cdict);
    } else {
        auto m_stream = bitio::stream(mstate->data.pointer(), mstate->data.size());

        for (int i = 0; i < 0x100; i++) {
            auto row = dict[i];
            for (int j = 0; j < 0x100; j++) {
                row[j] = m_stream.read(0x40);
            }
        }

        if (training_mode) {
            // Dispose mstate after it is interpreted.
            auto focm = hzmodels::FirstOrderContextModel();

            focm.set_alphabet_size(0x100);
            // Now we contruct a First-Order-Context-Dictionary.
            for (uint64_t i = 0; i < length; i++) {
                focm.update(data[i], 32);
            }

            // Incremental training on FOCM
            for (int i = 0; i < 0x100; i++) {
                auto row = focm.get_dist(i);
                auto target = dict[i];
                for (int k = 0; k < 0x100; k++) {
                    target[k] += row[k];
                }
            }

            // Generate mstate object.
            mstate->data = rainman::ptr<uint8_t>(65537 << 3);

            auto n_stream = bitio::stream(mstate->data.pointer(), mstate->data.size());

            for (int i = 0; i < 0x100; i++) {
                auto row = dict[i];
                for (int k = 0; k < 0x100; k++) {
                    n_stream.write(row[k], 0x40);
                }
            }
        }

        _normalize(dict, cdict);
    }
}

rainman::ptr<HZ_MState> hzcodec::Victini::train(const rainman::ptr<HZ_Blob> &blob) {
    auto mstate = blob->mstate;
    auto length = blob->o_size;

    auto data = rainman::ptr<int16_t>(length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->data[i];
    }

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(data, 0x100);
    bwt.transform();

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100);
    mtf.transform();

    auto dict = rainman::make_ptr2d<uint64_t>(0x100, 0x100);
    auto cdict = rainman::make_ptr2d<uint64_t>(0x100, 0x100);

    // Manage mstate
    _model_transform(mstate, dict, cdict, data, length, true);
    mstate->alg = hzcodec::algorithms::VICTINI;

    return mstate;
}

void hzcodec::Victini::_normalize(const rainman::ptr2d<uint64_t> &dict, const rainman::ptr2d<uint64_t> &cdict) {
    for (int i = 0; i < 0x100; i++) {
        auto row = dict[i];
        uint64_t sum = 0;
        for (int k = 0; k < 0x100; k++) {
            sum += row[k];
        }

        if (sum == 0) {
            sum = 1;
        }

        uint64_t dsum = 0;
        for (int k = 0; k < 0x100; k++) {
            row[k] = 1 + (row[k] * 16776960 / sum);
            dsum += row[k];
        }
        dsum = 16777216 - dsum;
        for (int k = 0; dsum > 0; k = (k + 1) & 0xff, dsum--) {
            row[k]++;
        }

        sum = 0;
        auto crow = cdict[i];
        for (int k = 0; k < 0x100; k++) {
            crow[k] = sum;
            sum += row[k];
        }
    }
}
