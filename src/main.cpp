#include <iostream>
#include <hzip/core/preprocessor/png_codec.h>
#include <hzip/core/models/pivoted_gaussian_model.h>
#include <hzip/core/models/single_order_context_model.h>
#include <hzip/core/preprocessor/burrows_wheeler_transform.h>
#include <hzip/core/preprocessor/move_to_front_transform.h>
#include <hzip/core/compressors/dark_ninja_compressor.h>
#include <hzip/core/compressors/white_rose_compressor.h>
#include <hzip/bitio/bitio.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (WhiteRoseCompression Test)" << std::endl;
    std::string filename = "/home/supercmmetry/Documents/dickens/dickens";

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto rose = WhiteRoseCompressor(filename);
    rose.compress(filename + ".hz");

    std::cout << "Time taken to compress: " << (float) (clock.now() - start).count() / 1000000000.0f << std::endl;
    start = clock.now();
    rose = WhiteRoseCompressor(filename + ".hz");
    rose.decompress(filename + ".hz.txt");
    std::cout << "Time taken to decompress: " << (float) (clock.now() - start).count() / 1000000000.0f << std::endl;
    return 0;
}