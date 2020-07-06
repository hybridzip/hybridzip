#include <iostream>
#include <hzip/core/compressors/white_rose.h>
#include <hzip/utils/utils.h>


int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;

    auto rose = hzcodec::white_rose("/home/supercmmetry/Documents/dickens/dickens");
    rose.compress("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.set_file("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.decompress("/home/supercmmetry/Documents/dickens/dickens.hz.txt");

    return 0;
}