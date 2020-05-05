#include <iostream>
#include <hzip/core/compressors/compressors.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;
    std::string filename = "/home/supercmmetry/Documents/dickens/dickens";

    auto rose = hzcodec::white_rose(filename);
    rose.compress(filename + ".hz");
    rose.set_file(filename + ".hz");
    rose.decompress(filename + ".hz.txt");

    return 0;
}