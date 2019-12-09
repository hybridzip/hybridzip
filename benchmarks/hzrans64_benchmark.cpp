#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>
#include "../core/entropy-coders/hzrans64.h"

#define SIZE 1024
#define BUFFER_SIZE 2ull << 20ull

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
    uint32_t *data = new uint32_t[BUFFER_SIZE];
    data += BUFFER_SIZE;

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
    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;


    //now we write data to file.
    FILE *out_fp = fopen("E:/dickens.hzf", "wb");
    fwrite(data, sizeof(uint32_t), state.count, out_fp);
    fclose(out_fp);
    //now we read the file.
    //reset data.
    data = new uint32_t[BUFFER_SIZE];
    FILE *in_fp = fopen("E:/dickens.hzf", "rb");
    fread(data, sizeof(uint32_t), state.count, in_fp); //state.count is constant for stride-based encoder..

    start = clock.now();
    //decoder ...
    hzrans64_codec_init(&state, 0x100, 12);
    hzrans64_dec_load_state(&state, &data);
    uint8_t sym;

    for (int i = 0; i < 0x100; i++) {
        dist[i] = 1;
    }
    sum = 0x100;
    for (int i = 0; i < SIZE; i++) {
        hzrans64_decode_s(&state, dist, i, &data, &sym);
        //std::cout << sym;
        dist[sym]++;
        sum++;
    }

    std::cout << "Time taken (decoder): " << (clock.now() - start).count() << " ns" << std::endl;
    return 0;
}