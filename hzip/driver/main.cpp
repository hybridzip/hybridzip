#include <iostream>
#include <random>
#include <chrono>
#include <cstring>


#define HZRANS_USE_AVX 0

#include "../core/blob/hzblobpack.h"
#include "../core/blob/hzbatchprocessor.h"


#define SIZE 1048576 * 9
#define FILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens"
#define OFILENAME "/run/media/supercmmetry/SYMPIENT/dickens/dickens.hz"


int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen(FILENAME, "rb");
    auto *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    int32_t dist[0x100];
    for (int & i : dist) i = 1;

    auto callback = [](uint8_t byte, int32_t *ptr) {
        ptr[byte]++;
    };

    hzBatchProcessor batchProc(0x100000, 12, strdup(FILENAME),
            strdup(OFILENAME));

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();


    batchProc.compress_batch(callback);


    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;

    bitio::bitio_stream rstream(strdup(OFILENAME), bitio::READ, 1024);
    hzBlobUnpacker unpacker;
    auto set = unpacker.unpack(rstream);


    return 0;
}