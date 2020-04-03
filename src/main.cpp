#include <iostream>
#include <hzip/core/preprocessor/png_codec.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (SuperPixel Remuxer TEST)" << std::endl;
    std::string filename = "/home/supercmmetry/Pictures/supercmmetry.png";
    auto codec = PNGCodec(filename);
    auto pixar = codec.read_rgb_pixels();

    return 0;
}