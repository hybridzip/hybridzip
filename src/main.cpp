#include <iostream>
#include <chrono>
#include <cstring>
#include <cmath>

#define HZRANS_USE_AVX 0

#include <hzip/bitio/bitio.h>
#include <hzip/core/blob/hzmthread.h>
#include <hzip/utils/boost_utils.h>
#include <hzip/core/blob/hzblobpack.h>
#include <hzip/utils/light_stack.h>
#include <hzip/core/preprocessor/burrows_wheeler_transform.h>
#include <hzip/core/preprocessor/move_to_front_transform.h>
#include <hzip/core/models/single_order_context_model.h>
#include <string>

int main() {
    std::cout << "hzip-unit-test started ... TEXT-COMPRESSOR (BWT)" << std::endl;
    std::string filename;
    std::cout << "Enter filename: " << std::endl;
    std::cin >> filename;
    hzboost::deleteFileIfExists(filename + ".hz");


    // PMM
    auto socm = SingleOrderContextModel(0x100);

    auto callback = [&socm](uint64_t byte, uint64_t *ptr) {
        auto *preds = socm.predict(byte);
        for (int i = 0; i < 0x100; i++) {
            ptr[i] = preds[i];
        }
        socm.update(byte);
    };

    // transform data to bwt-data


    auto proc = HZUProcessor(1);
    proc.setCallback(callback);
    proc.setBufferSize(hzboost::getFileSize(filename));

    auto *ces = new hz_cross_encoder;

    *ces = [](hzrans64_t *state, light_stack<uint32_t> *data) {};
    proc.setCrossEncoders(ces);

    auto *extractors = new std::function<uint64_t(void)>;

    auto *temp_stream = new bitio::bitio_stream(filename, bitio::READ, 1024);

    auto *data = new int[hzboost::getFileSize(filename)];
    uint64_t j = 0;
    while (!temp_stream->isEOF()) {
        data[j++] = temp_stream->read(0x8);
    }

    auto transformer = BurrowsWheelerTransformer(data, j, 0x100);
    auto bwt_index = transformer.transform();

    auto mtf = MoveToFrontTransformer(data, 0x100, j);
    mtf.transform();

    j = 0;
    *extractors = [data, &j]() {
        return data[j++];
    };

    proc.setExtractors(extractors);
    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    hzrblob_set set = proc.encode();

    std::cout << "Time taken for encoding: " << (float)(clock.now() - start).count() / 1000000000.0f << std::endl;

    auto ostream = bitio::bitio_stream(filename + ".hz", bitio::WRITE, 1024);
    hzBlobPacker packer;
    packer.pack(set);
    packer.commit(ostream);
    set.destroy();

    ostream.close();
    ostream = bitio::bitio_stream(filename + ".hz", bitio::READ, 1024);
//
//    hzBlobUnpacker unpacker;
//    set = unpacker.unpack(&ostream);
//
//    ostream.close();
//
//    start = clock.now();
//
//    auto vec = proc.decode(set);
//
//    std::cout << "Time taken for decoding: " << (float)(clock.now() - start).count() / 1000000000.0f << std::endl;
//    set.destroy();
//
//    auto o2stream = bitio::bitio_stream(filename + ".hz.orig", bitio::WRITE, 1024);
//    for(auto iter: vec) {
//        o2stream.write(iter, 0x8);
//    }
//
//    o2stream.flush();
//    o2stream.close();
    return 0;
}