#ifndef HYBRIDZIP_WHITE_ROSE_COMPRESSOR_H
#define HYBRIDZIP_WHITE_ROSE_COMPRESSOR_H

#include <string>
#include <other/bitio/bitio.h>
#include <hzip/utils/boost_utils.h>
#include <hzip/core/models/single_order_context_model.h>
#include <hzip/core/entropy/hzrans/hzrmthread.h>
#include <hzip/core/preprocessor/burrows_wheeler_transform.h>
#include <hzip/core/preprocessor/move_to_front_transform.h>
#include <hzip/core/blob/hzblobpack.h>

class WhiteRoseCompressor {
private:
    bitio::bitio_stream *stream;
public:
    WhiteRoseCompressor(std::string filename) {
        // use a 1MB buffer.
        stream = new bitio::bitio_stream(filename, bitio::READ, 1048576);
    }

    WhiteRoseCompressor() {
        // empty-constructor
    };

    void set_file(std::string filename) {
        // use a 1MB buffer.
        stream = new bitio::bitio_stream(filename, bitio::READ, 1048576);
    }

    void compress(std::string out_file_name) {
        hzboost::delete_file_if_exists(out_file_name);
        auto socm = SingleOrderContextModel(0x100);

        auto *data = new int[stream->get_file_size()];
        uint64_t j = 0;
        while (!stream->is_eof()) {
            data[j++] = stream->read(0x8);
        }
        auto length = j;

        auto bwt = BurrowsWheelerTransformer(data, length, 0x100);
        auto bwt_index = bwt.transform();

        auto mtf = MoveToFrontTransformer(data, 0x100, length);
        mtf.transform();

        // Now we contruct a First-Order-Context-Dictionary.
        for (uint64_t i = 0; i < length; i++) {
            socm.update(data[i], 32);
        }
        // Now we perform a one-time normalization for all possible contexts to increase speed.
        // Populate p-values and cumulative values.
        uint64_t dict[256][256];
        uint64_t dict_f[256][256];
        uint64_t cdict[256][256];

        for (int i = 0; i < 0x100; i++) {
            auto row = socm.predict(i);
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

        auto proc = HZUProcessor(1);
        proc.set_header(0x100, 24);
        proc.set_buffer_size(stream->get_file_size());
        proc.set_extractors(extractors);
        proc.bypass_normalization();

        auto *ces = new hz_cross_encoder;
        int64_t index = length;
        *ces = [dict, cdict, &index, data](hzrans64_t *state, light_stack<uint32_t> *_data) {
            index--;
            if (index != 0) {
                state->ls[index] = dict[data[index - 1]][data[index]];
                state->bs[index] = cdict[data[index - 1]][data[index]];
            } else {
                state->ls[index] = 65536;
                state->bs[index] = (((uint64_t) data[index] + 1)) << 16;
            }
        };

        proc.set_cross_encoders(ces);

        hzrblob_set set = proc.encode();

        auto ostream = bitio::bitio_stream(out_file_name, bitio::WRITE, 1048576);

        hzBlobPacker packer;
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
        set.destroy();
        ostream.close();
    }

    void decompress(std::string out_file_name) {
        hzboost::delete_file_if_exists(out_file_name);
        auto clock = std::chrono::high_resolution_clock();
        auto start = clock.now();

        auto proc = HZUProcessor(1);
        proc.set_header(0x100, 24);
        proc.set_buffer_size(stream->get_file_size());
        proc.bypass_normalization();


        hzBlobUnpacker unpacker(stream);

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
        auto *ces = new hz_cross_encoder;
        *ces = [dict, cdict, &index, &prev_symbol](hzrans64_t *state, light_stack<uint32_t> *data) {
            uint64_t x = state->x;
            uint64_t bs = x & state->mask;
            uint8_t symbol = 0;

            if (prev_symbol == -1) {
                for (int i = 0; i < 0x100; i++) {
                    if (((i + 1) << 16) > bs) {
                        symbol = --i;
                        break;
                    }
                }
                state->ls[index] = 65536;
                state->bs[index] = (symbol + 1) << 16;
            } else {
                for (int i = 0; i < 0x100; i++) {
                    if (cdict[prev_symbol][i] > bs) {
                        symbol = --i;
                        break;
                    }
                }
                state->ls[index] = dict[prev_symbol][symbol];
                state->bs[index] = cdict[prev_symbol][symbol];
            }
            prev_symbol = symbol;
            index++;
        };

        proc.set_cross_encoders(ces);

        // now we define a symbol callback.
        auto symbol_callback = [&prev_symbol]() {
            return prev_symbol;
        };

        auto set = unpacker.unpack();

        stream->close();

        auto vec = proc.decode(set, symbol_callback);
        set.destroy();

        auto *data = new int[vec.size()];
        int i = 0;
        for (auto iter : vec) {
            data[i++] = iter;
        }
        auto length = i;
        vec.clear();

        auto mtf = MoveToFrontTransformer(data, 0x100, length);
        mtf.invert();

        auto bwt = BurrowsWheelerTransformer(data, length, 0x100);
        bwt.invert(bwt_index);

        auto ostream = new bitio::bitio_stream(out_file_name, bitio::WRITE, 1048576);
        ostream->force_write<int>(data, length);
    }
};

#endif
