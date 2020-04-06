#include <iostream>
#include <hzip/bitio/bitio.h>
#include <hzip/core/preprocessor/jpeg_codec.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (JPEG-Compression Test)" << std::endl;
    std::string filename = "/home/supercmmetry/Pictures/supercmmetry.jpg";
    auto codec = JPEGCodec(filename);
    auto coeffs = codec.read_coefficients();

    // print a 8x8 dct-block
    std::cout << std::endl;
    for (int channel = 0; channel < coeffs.nchannels; channel++) {
        for (int block_index = 0; block_index < coeffs.coeff_arrays[channel].size(); block_index++) {
            std::cout << "Block: " << block_index << std::endl;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    std::cout << coeffs.coeff_arrays[channel][block_index][i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    return 0;
}