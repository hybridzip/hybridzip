#include <iostream>
#include <chrono>
#include <cstring>

#define HZRANS_USE_AVX 0

#include <hzip/bitio/bitio.h>
#include <hzip/core/blob/hzmthread.h>
#include <hzip/utils/boost_utils.h>
#include <hzip/log/global_logger.h>

#define FILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens"
#define OFILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens.hz"
#define O2FILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens.orig.txt"


int main() {
    hzlogger = Logger(strdup("./log.txt"), false);
    hzlogger.log(hzlog::INFO, "hzip-unit-test started");


    hzboost::deleteFileIfExists(OFILENAME);
    hzboost::deleteFileIfExists(O2FILENAME);

    auto callback = [](uint64_t byte, uint64_t *ptr) {
        ptr[byte]++;
    };



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

    return 0;
}