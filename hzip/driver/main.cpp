#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>

#define HZRANS_USE_AVX 0
#include "../core/entropy-coders/hzrans/hzrans.h"


#define SIZE 24
#define BUFFER_SIZE 1ull << 20ull

int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen("E:/dickens/dickens", "rb");
    uint8_t *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    hzrans64_t state;
    hzrans64_codec_init(&state, 0x100, 12);
    int32_t dist[0x100];
    for (int i = 0; i < 0x100; i++) dist[i] = 1;



    hzrans64_alloc_frame(&state, SIZE);
    hzrByteEncoder hbenc(SIZE, 12);
    hzrByteDecoder hbdec(SIZE, 12);

    hbenc.setDistribution(dist);
    hbdec.setDistribution(dist);

    auto clock = std::chrono::high_resolution_clock();

    auto start = clock.now();

    //generate ftable, normalize and pass to rans encoder.
    for (int i = 0; i < SIZE; i++) {
        hbenc.normalize(array[i]);
        dist[array[i]]++;
    }

    auto dptr = hbenc.encodeBytes();

    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;


    //now we write data to file.
    FILE *out_fp = fopen("E:/dickens.hzf", "wb");
    fwrite(dptr.data, sizeof(uint32_t), dptr.n, out_fp);
    fclose(out_fp);
    //now we read the file.
    //reset data.
    uint32_t *data = new uint32_t[SIZE];
    FILE *in_fp = fopen("E:/dickens.hzf", "rb");
    fread(data, sizeof(uint32_t), dptr.n, in_fp);

    start = clock.now();
    //decoder ...

    for (int i = 0; i < 0x100; i++) {
        dist[i] = 1;
    }

    auto callback = [&dist](uint8_t symbol) {
        dist[symbol]++;
    };
    hbdec.decodeBytes(data, callback);

    std::cout << "Time taken (decoder): " << (clock.now() - start).count() << " ns" << std::endl;
    return 0;
}