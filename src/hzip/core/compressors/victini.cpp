#include "victini.h"

hzcodec::victini::victini(std::string filename) {
    // use a 1MB hz_buffer.
    __deprecated_bitio_stream = new bitio::bitio_stream(filename, bitio::READ, false, 1048576);
}

void hzcodec::victini::set_file(std::string filename) {
    // use a 1MB hz_buffer.
    __deprecated_bitio_stream = new bitio::bitio_stream(filename, bitio::READ, false, 1048576);
}

// deprecated
void hzcodec::victini::compress(std::string out_file_name) {
    fsutils::delete_file_if_exists(out_file_name);

    auto focm = hzmodels::first_order_context_model();
    HZ_MEM_INIT(focm);

    focm.set_alphabet_size(0x100);

    auto *data = HZ_MALLOC(int, __deprecated_bitio_stream->get_file_size());

    uint64_t j = 0;
    while (!__deprecated_bitio_stream->is_eof()) {
        data[j++] = __deprecated_bitio_stream->read(0x8);
    }
    auto length = j;

    auto bwt = hztrans::bw_transformer(data, length, 0x100);
    HZ_MEM_INIT(bwt);

    auto bwt_index = bwt.transform();

    auto mtf = hztrans::mtf_transformer(data, 0x100, length);
    mtf.transform();

    // Now we contruct a First-Order-Context-Dictionary.
    for (uint64_t i = 0; i < length; i++) {
        focm.update(data[i], 32);
    }
    // Now we perform a one-time normalization for all possible contexts to increase speed.
    // Populate p-values and cumulative values.
    uint64_t dict[256][256];
    uint64_t dict_f[256][256];
    uint64_t cdict[256][256];

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
        for (int k = 0; dsum > 0; k = (k + 1) % 0x100, dsum--) {
            dict[i][k]++;
        }

        sum = 0;
        for (int k = 0; k < 0x100; k++) {
            cdict[i][k] = sum;
            sum += dict[i][k];
        }
    }


    auto *extractors = new std::function<uint64_t(void)>;
    j = 0;
    *extractors = [data, &j]() {
        return data[j++];
    };

    auto proc = hzu_proc(1);
    HZ_MEM_INIT(proc);

    proc.set_header(0x100, 24);
    proc.set_buffer_size(__deprecated_bitio_stream->get_file_size());
    proc.set_extractors(extractors);
    proc.bypass_normalization();

    auto *ces = new hz_cross_codec;
    int64_t index = length;
    *ces = [dict, cdict, &index, data](hzrans64_t *state, hz_stack<uint32_t> *_data) {
        index--;
        if (index != 0) {
            state->ls = dict[data[index - 1]][data[index]];
            state->bs = cdict[data[index - 1]][data[index]];
        } else {
            state->ls = 65536;
            state->bs = (((uint64_t) data[index] + 1)) << 16;
        }
    };

    proc.set_cross_encoders(ces);

    hzrblob_set set = proc.encode();

    HZ_FREE(data);

    auto ostream = bitio::bitio_stream(out_file_name, bitio::WRITE, false, 1048576);

    hz_blob_packer packer;
    //At first we pack the BWT-index.
    packer.pack_header(unarypx_bin(bwt_index));
    // now we need to pack the first-order-context-dictionary.
    // check if the length exceeds the scale value. If it does, store the normalized dictionary to increase,
    // decompression speed, otherwise store the frequency-dictionary to increase compression ratio.
    bool store_norm_dict = false;
    if (length >= 16777216) {
        store_norm_dict = true;
    }
    packer.pack_header(unarypx_bin(store_norm_dict));

    for (int i = 0; i < 0x100; i++) {
        for (int k = 0; k < 0x100; k++) {
            if (store_norm_dict) {
                packer.pack_header(unarypx_bin(dict[i][k]));
            } else {
                packer.pack_header(unarypx_bin(dict_f[i][k]));
            }
        }
    }
    //Then pack the ans-encoded socm-mtf-bwt-data
    packer.pack(set);
    packer.commit(ostream);
    //set.destroy();
    ostream.close();
}

// deprecated
void hzcodec::victini::decompress(std::string out_file_name) {
    fsutils::delete_file_if_exists(out_file_name);
    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto proc = hzu_proc(1);
    HZ_MEM_INIT(proc);

    proc.set_header(0x100, 24);
    proc.set_buffer_size(__deprecated_bitio_stream->get_file_size());
    proc.bypass_normalization();


    hz_blob_unpacker unpacker(__deprecated_bitio_stream);

    auto lb_stream = [&unpacker](uint64_t n) { return unpacker.unpack_header(n); };

    uint64_t bwt_index = unaryinv_bin(lb_stream).obj;
    bool is_norm_dict = unaryinv_bin(lb_stream).obj;
    uint64_t dict[256][256];
    uint64_t cdict[256][256];

    // populate the dictionary.
    for (int i = 0; i < 0x100; i++) {
        for (int j = 0; j < 0x100; j++) {
            dict[i][j] = unaryinv_bin(lb_stream).obj;
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
            for (int k = 0; dsum > 0; k = (k + 1) % 0x100, dsum--) {
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
    uint64_t index = 0;
    int prev_symbol = -1;
    auto sym_optr = new uint64_t;
    auto *ces = new hz_cross_codec;
    *ces = [dict, cdict, &prev_symbol, sym_optr](hzrans64_t *state, hz_stack<uint32_t> *data) {
        uint64_t x = state->x;
        uint64_t bs = x & state->mask;
        uint8_t symbol = 0;

        if (prev_symbol == -1) {
            for (int i = 0; i < 0x100; i++) {
                if (((i + 1) << 16) > bs) {
                    symbol = i - 1;
                    break;
                }
            }
            state->ls = 65536;
            state->bs = (symbol + 1) << 16;
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

    proc.set_cross_encoders(ces);

    auto set = unpacker.unpack();

    __deprecated_bitio_stream->close();

    auto vec = proc.decode(set, sym_optr);
    //set.destroy();

    auto *data = HZ_MALLOC(int, vec.size());

    int i = 0;
    for (auto iter : vec) {
        data[i++] = iter;
    }
    auto length = i;
    vec.clear();

    auto mtf = hztrans::mtf_transformer(data, 0x100, length);
    mtf.invert();

    auto bwt = hztrans::bw_transformer(data, length, 0x100);
    HZ_MEM_INIT(bwt);

    bwt.invert(bwt_index);

    auto ostream = new bitio::bitio_stream(out_file_name, bitio::WRITE, false, 1048576);
    ostream->force_write<int>(data, length);

    HZ_FREE(data);
}

hzblob_t *hzcodec::victini::compress(hzblob_t *blob) {
    auto mstate = blob->mstate;
    // create a raw bitio_stream on blob data
    auto length = blob->o_size;

    auto *data = HZ_MALLOC(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        data[i] = blob->o_data[i];
    }

    auto bwt = hztrans::bw_transformer(data, length, 0x100);
    HZ_MEM_INIT(bwt);

    auto bwt_index = bwt.transform();

    auto mtf = hztrans::mtf_transformer(data, 0x100, length);
    mtf.transform();

    auto *dict = HZ_MALLOC(uint64_t*, 256);
    auto *cdict = HZ_MALLOC(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = HZ_MALLOC(uint64_t, 256);
        cdict[i] = HZ_MALLOC(uint64_t, 256);
    }

    generate_mstate(mstate, dict, cdict, data, length, bwt_index);

    // Perform cross-encoding.
    uint64_t index = length;

    auto cross_encoder = [dict, cdict, &index, data](hzrans64_t *state, hz_stack<uint32_t> *_data) {
        index--;
        if (index != 0) {
            state->ls = dict[data[index - 1]][data[index]];
            state->bs = cdict[data[index - 1]][data[index]];
        } else {
            state->ls = 65536;
            state->bs = (((uint64_t) data[index] + 1)) << 16;
        }
    };

    auto encoder = hzu_encoder();
    HZ_MEM_INIT(encoder);

    encoder.set_header(0x100, 24, length);
    encoder.set_distribution(hzip_get_init_dist(HZ_MEM_MGR, 0x100));
    encoder.set_cross_encoder(cross_encoder);
    encoder.set_size(length);

    u32ptr blob_data = encoder.encode();

    for (int i = 0; i < 256; i++) {
        HZ_FREE(dict[i]);
        HZ_FREE(cdict[i]);
    }

    HZ_FREE(dict);
    HZ_FREE(cdict);
    HZ_FREE(data);

    auto cblob = HZ_NEW(hzblob_t);
    HZ_MEM_INIT_PTR(cblob);

    cblob->data = blob_data.data;
    cblob->size = blob_data.n;
    cblob->o_size = length;
    cblob->alg = hzcodec::algorithms::VICTINI;
    cblob->mstate = mstate;

    return cblob;
}

hzblob_t *hzcodec::victini::decompress(hzblob_t *blob) {
    auto mstate = blob->mstate;
    uint64_t length = blob->o_size;

    // Parse mstate.
    uint64_t k = 0;

    uint64_t bwt_index = mstate->bins[k++];
    bool is_norm_dict = mstate->bins[k++];

    auto *dict = HZ_MALLOC(uint64_t*, 256);
    auto *cdict = HZ_MALLOC(uint64_t*, 256);

    for (int i = 0; i < 256; i++) {
        dict[i] = HZ_MALLOC(uint64_t, 256);
        cdict[i] = HZ_MALLOC(uint64_t, 256);
    }


    // populate the dictionary.
    for (int i = 0; i < 0x100; i++) {
        for (int j = 0; j < 0x100; j++) {
            dict[i][j] = mstate->bins[k++];
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
            for (int k = 0; dsum > 0; k = (k + 1) % 0x100, dsum--) {
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
    auto sym_optr = HZ_NEW(uint64_t);

    auto cross_decoder = [dict, cdict, &prev_symbol, sym_optr](hzrans64_t *state, hz_stack<uint32_t> *data) {
        uint64_t x = state->x;
        uint64_t bs = x & state->mask;
        uint8_t symbol = 0;

        if (prev_symbol == -1) {
            for (int i = 0; i < 0x100; i++) {
                if (((i + 1) << 16) > bs) {
                    symbol = i - 1;
                    break;
                }
            }
            state->ls = 65536;
            state->bs = (symbol + 1) << 16;
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
    HZ_MEM_INIT(decoder);

    decoder.set_header(0x100, 24, length);
    decoder.set_cross_decoder(cross_decoder);
    decoder.set_distribution(hzip_get_init_dist(HZ_MEM_MGR, 0x100));
    decoder.override_symbol_ptr(sym_optr);

    auto dataptr = decoder.decode(blob->data);

    for (int i = 0; i < 256; i++) {
        HZ_FREE(dict[i]);
        HZ_FREE(cdict[i]);
    }

    HZ_FREE(dict);
    HZ_FREE(cdict);
    HZ_FREE(sym_optr);

    auto *sdata = HZ_MALLOC(int16_t, length);

    for (uint64_t i = 0; i < length; i++) {
        sdata[i] = dataptr.data[i];
    }

    free(dataptr.data);

    auto mtf = hztrans::mtf_transformer(sdata, 0x100, length);
    mtf.invert();

    auto bwt = hztrans::bw_transformer(sdata, length, 0x100);
    HZ_MEM_INIT(bwt);

    bwt.invert(bwt_index);

    auto dblob = HZ_NEW(hzblob_t);
    HZ_MEM_INIT_PTR(dblob);

    dblob->o_data = HZ_MALLOC(uint8_t, length);
    dblob->o_size = length;
    dblob->mstate = mstate;
    dblob->alg = hzcodec::algorithms::VICTINI;

    for (uint64_t i = 0; i < length; i++) {
        dblob->o_data[i] = sdata[i];
    }


    HZ_FREE(sdata);

    return dblob;
}

void hzcodec::victini::generate_mstate(hz_mstate *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data,
                                       uint64_t length, uint64_t bwt_index) {
    if (mstate->is_empty()) {
        auto focm = hzmodels::first_order_context_model();
        HZ_MEM_INIT(focm);

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
            for (int k = 0; dsum > 0; k = (k + 1) % 0x100, dsum--) {
                dict[i][k]++;
            }

            sum = 0;
            for (int k = 0; k < 0x100; k++) {
                cdict[i][k] = sum;
                sum += dict[i][k];
            }
        }


        // Generate mstate object.

        mstate->length = 65538;
        mstate->bins = HZ_MALLOC(uint64_t, mstate->length);

        bool store_norm_dict = false;

        if (length >= 16777216) {
            store_norm_dict = true;
        }


        int b_index = 0;
        mstate->bins[b_index++] = bwt_index;
        mstate->bins[b_index++] = store_norm_dict;

        for (int i = 0; i < 0x100; i++) {
            for (int k = 0; k < 0x100; k++) {
                if (store_norm_dict) {
                    mstate->bins[b_index++] = dict[i][k];
                } else {
                    mstate->bins[b_index++] = dict_f[i][k];
                }
            }
        }

    } else {
        uint64_t b_index = 1;

        bool is_norm_dict = mstate->bins[b_index++];

        for (int i = 0; i < 0x100; i++) {
            for (int j = 0; j < 0x100; j++) {
                dict[i][j] = mstate->bins[b_index++];
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
                for (int k = 0; dsum > 0; k = (k + 1) % 0x100, dsum--) {
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
