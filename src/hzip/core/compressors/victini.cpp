#include "victini.h"
#include <hzip/utils/utils.h>

HZ_Blob *hzcodec::Victini::compress(HZ_Blob *blob) {
    if (blob->mstate == nullptr) {
        blob->mstate = rxnew(HZ_MState);
    }

    auto mstate = blob->mstate;
    auto header = blob->header;
    auto length = blob->o_size;

    auto *data = rmalloc(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->data[i];
    }

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(data, length, 0x100);
    rinit(bwt);

    auto bwt_index = bwt.transform();

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100, length);
    mtf.transform();

    auto *dict = rmalloc(uint64_t*, 256);
    auto *cdict = rmalloc(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = rmalloc(uint64_t, 256);
        cdict[i] = rmalloc(uint64_t, 256);
    }

    // Write blob-header.
    header.raw = rmalloc(uint8_t, 8);
    header.length = 8;

    hz_u64_to_u8buf(bwt_index, header.raw);

    // Manage mstate
    gen_model_from_mstate(mstate, dict, cdict, data, length);

    // Perform cross-encoding.
    uint64_t index = length;

    auto cross_encoder = [dict, cdict, &index, data](hzrans64_t *state, HZ_Stack<uint32_t> *_data) {
        index--;
        if (index != 0) {
            state->ls = dict[data[index - 1]][data[index]];
            state->bs = cdict[data[index - 1]][data[index]];
        } else {
            state->ls = 65536;
            state->bs = ((uint64_t) data[index]) << 16;
        }
    };

    auto encoder = ronew(hzrans64_encoder);

    encoder.set_header(0x100, 24, length);
    encoder.set_distribution(hzip_get_init_dist(rmemmgr, 0x100));
    encoder.set_cross_encoder(cross_encoder);
    encoder.set_size(length);

    u32ptr blob_data = encoder.encode();

    for (int i = 0; i < 256; i++) {
        rfree(dict[i]);
        rfree(cdict[i]);
    }

    rfree(dict);
    rfree(cdict);
    rfree(data);

    auto cblob = rxnew(HZ_Blob);

    cblob->data = u32_to_u8ptr(rmemmgr, blob_data.data, blob_data.n);
    cblob->size = blob_data.n << 2;
    cblob->o_size = length;
    cblob->mstate = mstate;
    cblob->mstate->alg = hzcodec::algorithms::VICTINI;
    cblob->header = header;

    rfree(blob_data.data);
    return cblob;
}

HZ_Blob *hzcodec::Victini::decompress(HZ_Blob *blob) {
    auto mstate = blob->mstate;
    uint64_t length = blob->o_size;

    // Parse blob_header
    uint64_t bwt_index = hz_u8buf_to_u64(blob->header.raw);

    auto m_stream = new bitio::stream(mstate->data, mstate->length);

    auto *dict = rmalloc(uint64_t*, 256);
    auto *cdict = rmalloc(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = rmalloc(uint64_t, 256);
        cdict[i] = rmalloc(uint64_t, 256);
    }


    // populate the dictionary.
    for (int i = 0; i < 0x100; i++) {
        for (int j = 0; j < 0x100; j++) {
            dict[i][j] = m_stream->read(0x40);
        }
    }

    delete m_stream;

    // Normalize dictionary.
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
            dict[i][k] = 1 + (row[k] * 16776960 / sum);
            dsum += dict[i][k];
        }
        dsum = 16777216 - dsum;
        for (int k = 0; dsum > 0; k = (k + 1) & 0xff, dsum--) {
            dict[i][k]++;
        }
    }


    // populate cumulative values.
    for (int i = 0; i < 0x100; i++) {
        uint64_t sum = 0;
        for (int k = 0; k < 0x100; k++) {
            cdict[i][k] = sum;
            sum += dict[i][k];
        }
    }

    // create a cross-decoder.
    // a cross-decoder usually handles add_to_seq for the core entropy codec.
    int prev_symbol = -1;
    auto sym_optr = rnew(uint64_t);

    auto cross_decoder = [dict, cdict, &prev_symbol, sym_optr](hzrans64_t *state, HZ_Stack<uint32_t> *data) {
        uint64_t x = state->x;
        uint64_t bs = x & state->mask;
        uint8_t symbol = 0;

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
    rinit(decoder);

    decoder.set_header(0x100, 24, length);
    decoder.set_cross_decoder(cross_decoder);
    decoder.set_distribution(hzip_get_init_dist(rmemmgr, 0x100));
    decoder.override_symbol_ptr(sym_optr);

    uint32_t *blob_data = u8_to_u32ptr(rmemmgr, blob->data, blob->size);

    auto dataptr = decoder.decode(blob_data);

    for (int i = 0; i < 256; i++) {
        rfree(dict[i]);
        rfree(cdict[i]);
    }

    rfree(dict);
    rfree(cdict);
    rfree(sym_optr);
    rfree(blob_data);

    auto *sdata = rmalloc(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        sdata[i] = dataptr.data[i];
    }

    rfree(dataptr.data);

    auto mtf = hztrans::MoveToFrontTransformer(sdata, 0x100, length);
    mtf.invert();

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(sdata, length, 0x100);
    rinit(bwt);

    bwt.invert(bwt_index);

    auto dblob = rxnew(HZ_Blob);

    dblob->data = rmalloc(uint8_t, length);
    dblob->o_size = length;
    dblob->mstate = mstate;
    dblob->mstate->alg = hzcodec::algorithms::VICTINI;

    for (uint64_t i = 0; i < length; i++) {
        dblob->data[i] = sdata[i];
    }


    rfree(sdata);

    return dblob;
}

void hzcodec::Victini::gen_model_from_mstate(HZ_MState *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data,
                                             uint64_t length, bool training_mode) {
    if (mstate->is_empty()) {
        auto focm = hzmodels::FirstOrderContextModel();
        rinit(focm);

        focm.set_alphabet_size(0x100);
        // Now we contruct a First-Order-Context-Dictionary.
        for (uint64_t i = 0; i < length; i++) {
            focm.update(data[i], 32);
        }

        // Now we perform a one-time normalization for all possible contexts to increase speed.
        // Populate p-values and cumulative values.
        uint64_t dict_f[256][256];

        for (int i = 0; i < 0x100; i++) {
            auto row = focm.get_dist(i);
            uint64_t sum = 0;
            for (int k = 0; k < 0x100; k++) {
                sum += row[k];
                dict_f[i][k] = row[k];
            }

            if (sum == 0) {
                sum = 0x1;
            }

            uint64_t dsum = 0;
            for (int k = 0; k < 0x100; k++) {
                dict[i][k] = 1 + (row[k] * 16776960 / sum);
                dsum += dict[i][k];
            }
            dsum = 16777216 - dsum;
            for (int k = 0; dsum > 0; k = (k + 1) & 0xff, dsum--) {
                dict[i][k]++;
            }

            sum = 0;
            for (int k = 0; k < 0x100; k++) {
                cdict[i][k] = sum;
                sum += dict[i][k];
            }
        }


        // Generate mstate object.
        mstate->length = 65537 << 3;
        mstate->data = rmalloc(uint8_t, mstate->length);

        auto m_stream = new bitio::stream(mstate->data, mstate->length);

        for (auto &i : dict_f) {
            for (unsigned long k : i) {
                m_stream->write(k, 0x40);
            }
        }

        delete m_stream;

    } else {
        auto m_stream = new bitio::stream(mstate->data, mstate->length);

        for (int i = 0; i < 0x100; i++) {
            for (int j = 0; j < 0x100; j++) {
                dict[i][j] = m_stream->read(0x40);
            }
        }

        delete m_stream;

        if (training_mode) {
            // Dispose mstate after it is interpreted.
            mstate->destroy();

            auto focm = hzmodels::FirstOrderContextModel();
            rinit(focm);

            focm.set_alphabet_size(0x100);
            // Now we contruct a First-Order-Context-Dictionary.
            for (uint64_t i = 0; i < length; i++) {
                focm.update(data[i], 32);
            }

            // Incremental training on FOCM
            for (int i = 0; i < 0x100; i++) {
                auto row = focm.get_dist(i);
                for (int k = 0; k < 0x100; k++) {
                    dict[i][k] += row[k];
                }
            }

            // Generate mstate object.
            mstate->length = 65537 << 3;

            mstate->data = rmalloc(uint8_t, mstate->length);

            auto m_stream = new bitio::stream(mstate->data, mstate->length);

            for (int i = 0; i < 0x100; i++) {
                for (int k = 0; k < 0x100; k++) {
                    m_stream->write(dict[i][k], 0x40);
                }
            }

            delete m_stream;
        }


        for (int i = 0; i < 0x100; i++) {
            auto row = dict[i];
            uint64_t sum = 0;
            for (int k = 0; k < 0x100; k++) {
                sum += row[k];
            }

            if (sum == 0) {
                sum = 0x1;
            }

            uint64_t dsum = 0;
            for (int k = 0; k < 0x100; k++) {
                dict[i][k] = 1 + (row[k] * 16776960 / sum);
                dsum += dict[i][k];
            }
            dsum = 16777216 - dsum;
            for (int k = 0; dsum > 0; k = (k + 1) & 0xff, dsum--) {
                dict[i][k]++;
            }

            sum = 0;
            for (int k = 0; k < 0x100; k++) {
                cdict[i][k] = sum;
                sum += dict[i][k];
            }
        }
    }
}

HZ_MState *hzcodec::Victini::train(HZ_Blob *blob) {
    if (blob->mstate == nullptr) {
        blob->mstate = rxnew(HZ_MState);
    }

    auto mstate = blob->mstate;
    auto length = blob->o_size;

    auto *data = rmalloc(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->data[i];
    }

    auto bwt = hztrans::BurrowsWheelerTransformer<int16_t, int32_t>(data, length, 0x100);
    rinit(bwt);

    bwt.transform();

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100, length);
    mtf.transform();

    auto *dict = rmalloc(uint64_t*, 256);
    auto *cdict = rmalloc(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = rmalloc(uint64_t, 256);
        cdict[i] = rmalloc(uint64_t, 256);
    }

    // Manage mstate
    gen_model_from_mstate(mstate, dict, cdict, data, length, true);

    for (int i = 0; i < 256; i++) {
        rfree(dict[i]);
        rfree(cdict[i]);
    }

    rfree(dict);
    rfree(cdict);
    rfree(data);

    mstate->alg = hzcodec::algorithms::VICTINI;

    return mstate;
}
