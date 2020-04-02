#include <iostream>
#include <chrono>
#include <cstring>
#include <cmath>

#include <hzip/utils/boost_utils.h>
#include <hzip/core/compressors/dark_ninja_compressor.h>
#include <hzip/bitio/bitio.h>

int main() {
    std::cout << "hybridzip - v1.0.0" << std::endl;
//    std::cout << "Enter filename: ";
    std::string filename = "/home/supercmmetry/Documents/dickens/dickens";
//    std::cin >> filename;
//
    auto ninja = DarkNinjaCompressor(filename);
    ninja.compress(filename + ".hz");
    ninja.set_file(filename + ".hz");
    ninja.decompress(filename + ".hz.txt");
    return 0;
}