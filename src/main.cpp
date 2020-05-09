#include <iostream>
#include <hzip/core/compressors/compressors.h>
#include <hzip/compute/dct.h>
#include <hzip/compute/matrix_ops.h>
#include <immintrin.h>


int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;
    std::string filename = "/home/supercmmetry/Documents/dickens/dickens";

    auto *A = new double[0x40];
    auto *B = new double[0x40];

    for (int i = 0; i < 0x40; i++) {
        A[i] = i;
    }

    for (int i = 0; i < 8; i++) {
        hzcompute::dct::apply_dct8l_pd(A + 8 * i, B + 8 * i);
        hzcompute::dct::apply_idct81_pd(B + 8 * i, A + 8 * i);

        for (int j = 0; j < 0x8; j++) {
            if (j % 8 == 0) {
                std::cout << std::endl;
            }
            std::cout << round(A[8 * i + j]) << "\t";
        }
    }

    return 0;
}