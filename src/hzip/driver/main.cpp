#include <iostream>
#include <chrono>
#include <cstring>

#define HZRANS_USE_AVX 0

#include <hzip/bitio/bitio.h>
#include <hzip/core/blob/hzmthread.h>
#include <hzip/core/utils/boost_utils.h>

#define FILENAME "/run/media/supercmmetry/SYMPIENT/sample.txt"
#define OFILENAME "/run/media/supercmmetry/SYMPIENT/sample.txt.hz"
#define O2FILENAME "/run/media/supercmmetry/SYMPIENT/sample.txt.orig"


int main() {
    std::cout << "hzrans64-unit-test" << std::endl;
    deleteFileIfExists(OFILENAME);
    deleteFileIfExists(O2FILENAME);

    auto callback = [](uint64_t byte, uint64_t *ptr) {
        ptr[byte]++;
    };

    auto rstream = bitio::bitio_stream(FILENAME, bitio::READ, 1024);

    auto n = 4;
    auto proc = HZUProcessor(n);
    proc.setCallback(callback);
    proc.setBufferSize(8);

    auto extractors = new std::function<uint64_t(void)>[n];

    for (int i = 0; i < n; i++) {
        auto temp_stream = new bitio::bitio_stream(FILENAME, bitio::READ, 1024);
        temp_stream->skip(16 * i);
        extractors[i] = [temp_stream]() {
            return temp_stream->read(0x8);
        };
    }

    proc.setExtractors(extractors);


    hzrblob_set set = proc.encode();
    auto vec = proc.decode(set);
    for (int i = 0; i < vec.size(); i++) {
        std::cout << (char) vec[i];
    }
    std::cout << std::endl;

    return 0;
}