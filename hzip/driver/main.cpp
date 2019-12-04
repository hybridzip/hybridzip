#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>

#define HZRANS_USE_AVX 1

#include "../core/blob/hzmthread.h"


#define SIZE 1048576
#define BUFFER_SIZE 1ull << 20ull

int main() {
    std::cout << "hzrans64-unit-test" << std::endl;

    //read 1KB from dickens...
    FILE *fp;
    fp = fopen("E:/dickens/dickens", "rb");
    uint8_t *array = new uint8_t[SIZE];
    fread(array, 1, SIZE, fp);

    int32_t dist[0x100];
    for (int i = 0; i < 0x100; i++) dist[i] = 1;

    hzMthByteBlob hzmbb(12, array, SIZE);
    hzmbb.setCallback([](uint8_t byte, int32_t *ptr) {
       ptr[byte]++;
    });


    auto clock = std::chrono::high_resolution_clock();

    auto start = clock.now();
    hzmbb.run();

    std::cout << "\n\nTime taken (encoder): " << (clock.now() - start).count() << " ns" << std::endl;

    return 0;
}