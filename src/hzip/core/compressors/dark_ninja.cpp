#include "dark_ninja.h"

hzcomp_dark_ninja::dark_ninja::dark_ninja(std::string filename) {
    // use a 1MB buffer.
    stream = new bitio::bitio_stream(filename, bitio::READ, false, 1048576);
}

void hzcomp_dark_ninja::dark_ninja::set_file(std::string filename) {
    // use a 1MB buffer.
    stream = new bitio::bitio_stream(filename, bitio::READ, false, 1048576);
}

void hzcomp_dark_ninja::dark_ninja::compress(std::string out_file_name) {
    hzboost::delete_file_if_exists(out_file_name);
    auto focm = hzmodels::first_order_context_model(0x100);
    auto callback = [&focm](uint64_t byte, uint64_t *ptr) {
        auto *preds = focm.predict(byte);
        for (int i = 0; i < 0x100; i++) {
            ptr[i] = preds[i];
        }
        focm.update(byte, 48);
    };


    auto proc = hzu_proc(1);
    proc.set_header(0x100, 24);
    proc.set_buffer_size(stream->get_file_size());
    proc.set_callback(callback);
    proc.use_only_base_encoder();

    auto *data = new int[stream->get_file_size()];
    uint64_t j = 0;
    while (!stream->is_eof()) {
        data[j++] = stream->read(0x8);
    }
    auto length = j;

    auto bwt = hz_trans::bw_transformer(data, length, 0x100);
    auto bwt_index = bwt.transform();

    auto mtf = hz_trans::mtf_transformer(data, 0x100, length);
    mtf.transform();

    auto *extractors = new std::function<uint64_t(void)>;
    j = 0;
    *extractors = [data, &j]() {
        return data[j++];
    };

    proc.set_extractors(extractors);

    hzrblob_set set = proc.encode();

    auto ostream = bitio::bitio_stream(out_file_name, bitio::WRITE, false, 1048576);

    hz_blob_packer packer;
    //At first we pack the BWT-index.
    packer.pack_header(unarypx_bin(bwt_index));
    //Then pack the ans-encoded focm-mtf-bwt-data
    packer.pack(set);
    packer.commit(ostream);
    set.destroy();
    ostream.close();
}

void hzcomp_dark_ninja::dark_ninja::decompress(std::string out_file_name) {
    hzboost::delete_file_if_exists(out_file_name);
    auto focm = hzmodels::first_order_context_model(0x100);

    auto callback = [&focm](uint64_t byte, uint64_t *ptr) {
        auto *preds = focm.predict(byte);
        for (int i = 0; i < 0x100; i++) {
            ptr[i] = preds[i];
        }
        focm.update(byte, 48);
    };

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto proc = hzu_proc(1);
    proc.set_header(0x100, 24);
    proc.set_buffer_size(stream->get_file_size());
    proc.set_callback(callback);
    proc.use_only_base_encoder();


    hz_blob_unpacker unpacker(stream);
    uint64_t bwt_index = unaryinv_bin([&unpacker](uint64_t n) { return unpacker.unpack_header(n); }).obj;
    auto set = unpacker.unpack();

    stream->close();

    auto vec = proc.decode(set);
    set.destroy();

    auto *data = new int[vec.size()];
    int i = 0;
    for (auto iter : vec) {
        data[i++] = iter;
    }
    auto length = i;
    vec.clear();

    auto mtf = hz_trans::mtf_transformer(data, 0x100, length);
    mtf.invert();

    auto bwt = hz_trans::bw_transformer(data, length, 0x100);
    bwt.invert(bwt_index);

    auto ostream = new bitio::bitio_stream(out_file_name, bitio::WRITE, false, 1048576);
    ostream->force_write<int>(data, length);
}

