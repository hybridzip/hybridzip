#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>
#include "../core/entropy-coders/hzrans64.h"

#define SIZE 1024

int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen("E:/dickens/dickens", "rb");
    uint8_t *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    hzrans64_t state;
    hzrans64_codec_init(&state, 0x100, 12);
    uint64_t dist[0x100];

    for (int i = 0; i < 0x100; i++) {
        dist[i] = 1;
        state.ftable[i] = (1ull << state.scale) / 0x100;
    }


    //now we run the rans encoder with a dynamic probability distribution
    uint64_t sum = 0x100;
    uint32_t *data = new uint32_t[1ull << 20ull]; //32mb
    data += 1ull << 20ull;

    hzrans64_alloc_frame(&state, SIZE);

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    //generate ftable, normalize and pass to rans encoder.

    for (int i = 0; i < SIZE; i++) {
        //std::cout << (int)array[i] << " ";
        hzrans64_create_ftable_nf(&state, dist);
        hzrans64_add_to_seq(&state, array[i], i);
        dist[array[i]]++;
        sum++;
    }
    //reverse-encode using ftable.
    for (int i = SIZE - 1; i >= 0; i--) {
        hzrans64_encode_s(&state, i, &data);
    }
    hzrans64_enc_flush(&state, &data);
    //std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;

    //decoder ...
    hzrans64_dec_init(&state, 0x100, 12);
    hzrans64_dec_load_final(&state, &data);
    uint8_t sym;

    for (int i = 0; i < 0x100; i++) {
        dist[i] = 1;
    }
    sum = 0x100;
    for (int i = 0; i < SIZE; i++) {
        hzrans64_decode_s(&state, dist, i, &data, &sym);
        dist[sym]++;
        sum++;
    }

    //start = clock.now();
    return 0;
}