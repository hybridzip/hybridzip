#include <iostream>
#include <random>
#include <chrono>
#include <cassert>

#define HZRANS_USE_AVX 1

#include "../core/blob/hzmthread.h"
#include "../core/blob/hzblobpack.h"
#include "../core/blob/hzbatchprocessor.h"

#define SIZE 1048576 * 9

int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen("E:/dickens/dickens", "rb");
    uint8_t *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    int32_t dist[0x100];
    for (int i = 0; i < 0x100; i++) dist[i] = 1;

    auto callback = [](uint8_t byte, int32_t *ptr) {
        ptr[byte]++;
    };

    hzBatchProcessor batchProc(0x400, 12, _strdup("E:/dickens/dickens"),
            _strdup("E:/dickens.hzbs"));

    auto clock = std::chrono::high_resolution_clock();
    bitio::bitio_stream bstream(_strdup("E:/dickens.hzbs"), bitio::WRITE, 1024);

    auto start = clock.now();
    batchProc.compress_batch(callback);
    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;

    bitio::bitio_stream rstream(_strdup("E:/dickens.hzbs"), bitio::READ, 1024);
    hzBlobUnpacker unpacker;
    auto set = unpacker.unpack(rstream);


    return 0;
}