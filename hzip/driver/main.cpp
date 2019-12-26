#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cassert>


#define HZRANS_USE_AVX 1

#include "../core/blob/hzmthread.h"
#include "../core/blob/hzblobpack.h"
#include "../core/blob/hzbatchprocessor.h"


#define SIZE 1048576 * 9
#define FILENAME "/run/media/supercmmetry/dickens/dickens"
#define OFILENAME "/run/media/supercmmetry/dickens/dickens.hz"
#define BUFFER_SIZE 1ull << 20ull


int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen(FILENAME, "rb");
    uint8_t *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    int32_t dist[0x100];
    for (int i = 0; i < 0x100; i++) dist[i] = 1;

    auto callback = [](uint8_t byte, int32_t *ptr) {
        ptr[byte]++;
    };

    hzBatchProcessor batchProc(0x400, 12, strdup(FILENAME),
            strdup(OFILENAME));

    auto clock = std::chrono::high_resolution_clock();
    bitio::bitio_stream bstream(strdup(OFILENAME), bitio::WRITE, 1024);

    auto start = clock.now();
    batchProc.compress_batch(callback);
    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;

    bitio::bitio_stream rstream(strdup(OFILENAME), bitio::READ, 1024);
    hzBlobUnpacker unpacker;
    auto set = unpacker.unpack(rstream);


    return 0;
}