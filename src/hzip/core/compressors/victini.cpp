#include "victini.h"
#include <hzip/utils/utils.h>

hzblob_t *hzcodec::victini::compress(hzblob_t *blob) {
    auto mstate = blob->mstate;
    auto header = blob->header;
    auto length = blob->o_size;

    auto *data = rmalloc(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->o_data[i];
    }

    auto bwt = hztrans::bw_transformer<int16_t, int32_t>(data, length, 0x100);
    rinit(bwt);

    auto bwt_index = bwt.transform();

    auto mtf = hztrans::mtf_transformer(data, 0x100, length);
    mtf.transform();

    auto *dict = rmalloc(uint64_t*, 256);
    auto *cdict = rmalloc(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = rmalloc(uint64_t, 256);
        cdict[i] = rmalloc(uint64_t, 256);
    }

    // Write blob-header.
    header.raw = rmalloc(uint8_t, 32);
    auto h_stream = new bitio::stream(header.raw, 32);

    bin_t bwt_index_bin = elias_gamma(bwt_index);
    h_stream->write(bwt_index_bin.obj, bwt_index_bin.n);
    h_stream->flush();

    header.length = h_stream->get_stream_size();

    delete h_stream;

    // Manage mstate
    gen_model_from_mstate(mstate, dict, cdict, data, length);

    // Perform cross-encoding.
    uint64_t index = length;

    auto cross_encoder = [dict, cdict, &index, data](hzrans64_t *state, hz_stack<uint32_t> *_data) {
        index--;
        if (index != 0) {
            state->ls = dict[data[index - 1]][data[index]];
            state->bs = cdict[data[index - 1]][data[index]];
        } else {
            state->ls = 65536;
            state->bs = ((uint64_t) data[index]) << 16;
        }
    };

    auto encoder = hzu_encoder();
    rinit(encoder);

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

    auto cblob = rnew(hzblob_t);
    rinitptr(cblob);

    cblob->data = blob_data.data;
    cblob->size = blob_data.n;
    cblob->o_size = length;
    cblob->mstate = mstate;
    cblob->mstate->alg = hzcodec::algorithms::VICTINI;
    cblob->header = header;

    return cblob;
}

hzblob_t *hzcodec::victini::decompress(hzblob_t *blob) {
    auto mstate = blob->mstate;
    uint64_t length = blob->o_size;

    // Parse blob_header using bitio
    auto h_stream = new bitio::stream(blob->header.raw, blob->header.length);

    uint64_t bwt_index = elias_gamma_inv([h_stream](uint64_t n) {
        return h_stream->read(n);
    }).obj;

    delete h_stream;

    auto m_stream = new bitio::stream(mstate->data, mstate->length);

    // Parse mstate.
    uint64_t k = 0;
    bool is_norm_dict = m_stream->read(0x1);

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
    // check if we need to normalize the dictionary.
    if (!is_norm_dict) {
        for (int i = 0; i < 0x100; i++) {
            auto row = dict[i];
            uint64_t sum = 0;
            for (int k = 0; k < 0x100; k++) {
                sum += row[k];
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

    auto cross_decoder = [dict, cdict, &prev_symbol, sym_optr](hzrans64_t *state, hz_stack<uint32_t> *data) {
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

    auto decoder = hzu_decoder();
    rinit(decoder);

    decoder.set_header(0x100, 24, length);
    decoder.set_cross_decoder(cross_decoder);
    decoder.set_distribution(hzip_get_init_dist(rmemmgr, 0x100));
    decoder.override_symbol_ptr(sym_optr);

    auto dataptr = decoder.decode(blob->data);

    for (int i = 0; i < 256; i++) {
        rfree(dict[i]);
        rfree(cdict[i]);
    }

    rfree(dict);
    rfree(cdict);
    rfree(sym_optr);

    auto *sdata = rmalloc(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        sdata[i] = dataptr.data[i];
    }

    free(dataptr.data);

    auto mtf = hztrans::mtf_transformer(sdata, 0x100, length);
    mtf.invert();

    auto bwt = hztrans::bw_transformer<int16_t, int32_t>(sdata, length, 0x100);
    rinit(bwt);

    bwt.invert(bwt_index);

    auto dblob = rnew(hzblob_t);
    rinitptr(dblob);


    dblob->o_data = rmalloc(uint8_t, length);
    dblob->o_size = length;
    dblob->mstate = mstate;
    dblob->mstate->alg = hzcodec::algorithms::VICTINI;

    for (uint64_t i = 0; i < length; i++) {
        dblob->o_data[i] = sdata[i];
    }


    rfree(sdata);

    return dblob;
}

void hzcodec::victini::gen_model_from_mstate(hz_mstate *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data,
                                             uint64_t length) {
    if (mstate->is_empty()) {
        auto focm = hzmodels::first_order_context_model();
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

        bool store_norm_dict = false;

        if (length >= 16777216) {
            store_norm_dict = true;
        }

        m_stream->write(store_norm_dict, 1);

        for (int i = 0; i < 0x100; i++) {
            for (int k = 0; k < 0x100; k++) {
                if (store_norm_dict) {
                    m_stream->write(dict[i][k], 0x40);
                } else {
                    m_stream->write(dict_f[i][k], 0x40);
                }
            }
        }

        delete m_stream;

    } else {
        auto m_stream = new bitio::stream(mstate->data, mstate->length);
        bool is_norm_dict = m_stream->read(0x1);

        for (int i = 0; i < 0x100; i++) {
            for (int j = 0; j < 0x100; j++) {
                dict[i][j] = m_stream->read(0x40);
            }
        }

        delete m_stream;

        // check if we need to normalize the dictionary.
        if (!is_norm_dict) {
            for (int i = 0; i < 0x100; i++) {
                auto row = dict[i];
                uint64_t sum = 0;
                for (int k = 0; k < 0x100; k++) {
                    sum += row[k];
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
}
