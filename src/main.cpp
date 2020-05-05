#include <iostream>
#include <hzip/core/preprocessor/jpeg_codec.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;


    auto codec = HZJPEGCodec("/home/supercmmetry/Pictures/supercmmetry.jpg");
    //codec.test();
    auto image = codec.read();

    codec.write(image, fopen("/home/supercmmetry/Pictures/supercmmetry.copy.jpg", "wb"));

    return 0;
}