#include <iostream>
#include <chrono>
#include <cstring>

#define HZRANS_USE_AVX 0
#include <hzip/core/blob/hzbatchprocessor.h>

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


    auto clock = std::chrono::high_resolution_clock();
    auto filesize = getFileSize(strdup(FILENAME));

    hzBatchProcessor batchProc(0x800000, 12, strdup(FILENAME),
            strdup(OFILENAME));

    auto start = clock.now();

    batchProc.compress_batch(callback);

    std::cout << "Compression rate: " << double(filesize * 1000000000 / 1024) / (clock.now() - start).count() << " KBps" << std::endl;


    batchProc.set_src_dest(strdup(OFILENAME), strdup(O2FILENAME));

    start = clock.now();
    batchProc.decompress_batch(callback);
    std::cout << "Decompression rate: " << double(filesize * 1000000000 / 1024) / (clock.now() - start).count() << " KBps" << std::endl;

    return 0;
}