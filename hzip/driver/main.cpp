#include <iostream>
#include <chrono>
#include <cstring>

#define HZRANS_USE_AVX 0
#include "../core/blob/hzbatchprocessor.h"

#define FILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens"
#define OFILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens.hz"
#define O2FILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens.orig"


int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

//    FILE *fp = fopen(OFILENAME, "rb");
//    fseek(fp, 604449, SEEK_CUR);
//    auto b = new char[1];
//
//    fread(b,1,1,fp);
//    std::cout << (b[0] & 0xff) << std::endl;
//    fread(b,1,1,fp);
//    std::cout << (b[0] & 0xff) << std::endl;
//
//    fclose(fp);
//
//
//    bitio::bitio_stream stream(OFILENAME, bitio::READ, HZ_BITIO_BUFFER_SIZE);
//
//    for(int i = 0; i < 604449; i++) {
//        stream.read(5);
//    }
//    for(int i = 0; i < 604449; i++) {
//        stream.read(3);
//    }
//
//    std::cout << stream.read(8) << std::endl;
//    std::cout << stream.read(8) << std::endl;

    int32_t dist[0x100];
    for (int & i : dist) i = 1;

    auto callback = [](uint8_t byte, int32_t *ptr) {
        ptr[byte]++;
    };

    hzBatchProcessor batchProc(0x100000, 12, strdup(FILENAME),
            strdup(OFILENAME));

    batchProc.compress_batch(callback);

    // reset distribution.
    for (int & i : dist) i = 1;
    batchProc.set_src_dest(strdup(OFILENAME), strdup(O2FILENAME));
    batchProc.decompress_batch(callback);

    return 0;
}