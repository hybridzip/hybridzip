#include <iostream>
#include <hzip/core/compressors/compressors.h>
#include <hzip/compute/dct.h>
#include <hzip/core/preprocessor/jpeg_codec.h>
#include <hzip/compute/matrix_ops.h>
#include <immintrin.h>


int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;
    std::string filename = "/home/supercmmetry/Pictures/supercmmetry.jpg";

    auto codec = hzj_iface(filename);
    auto image = codec.read();

    double freq[0x100][0x100];

    for (int i = 0; i < 0x100; i++) {
        for (int j = 0; j < 0x100; j++) {
            freq[i][j] = 1.0;
        }
    }

    int prev = 0;
    int M = 64;
    double bits = 0;
    double sum = 256;
    auto R = 0;

    //std::cout << "Estimating compressed size ..." << std::endl;

    int k = 2;
    int l = 2;
    int c = 0;


    for (k = 0; k < 2; k++) {
        for (l = 0; l < 2; l++) {
            if (k == 0 && l == 0) {
                continue;
            }

            for (int i = 0; i < 0x100; i++) {
                for (int j = 0; j < 0x100; j++) {
                    freq[i][j] = 1.0;
                }
            }

            sum = 256;
            prev = 0;

            for (int i = 0; i < M; i++) {
                for (int j = 0; j < M; j++) {
                    if (j % M == 0) {
                        // std::cout << std::endl;
                    }
                    if (j % 2 == 1 && j + 1 != M) {
                        auto avg = (float) image.mcus.coeff_arrays[c][M * i + (j - 1)][k][l] +
                                   (float) image.mcus.coeff_arrays[c][M * i + (j + 1)][k][l];
                        if (i != 0) {
                            avg += (float) (image.mcus.coeff_arrays[c][M * (i - 1) + j][k][l]);
                            avg /= 3;
                        } else {
                            avg /= 2;
                        }
//                avg /= 2;

                        int iavg = (int) avg;
                        auto xor_val = 128 + (image.mcus.coeff_arrays[c][M * i + j][k][l] ^ iavg);
                        //auto xor_val = 128 + image.mcus.coeff_arrays[c][M * i + j][k][l];
                        //std::cout <<  xor_val << "\t";
                        sum = 0;
                        for (int m = 0; m < 0x100; m++) {
                            sum += freq[prev][m];
                        }

                        bits += -log2(freq[prev][xor_val] / sum);
                        freq[prev][xor_val] += 32;
//                        prev = xor_val;

                    } else {
                        //std::cout << image.mcus.coeff_arrays[0][M * i + j][2][2] << "\t";
                    }
                }
            }
        }
    }


    std::cout << "Minimum compressed size: " << bits / 4 << " bytes" << std::endl;

    return 0;
}