#include <iostream>
#include "dct.h"
#include "matrix_ops.h"

#define DESCALE(n) n >>= 16

using namespace hzcompute;

void dct::apply_dct8x8_p16(int32_t *A, int32_t *res) {
    // res = P * A * transpose(P)
    int32_t *tmp = new int32_t[0x40];

    int32_t *dct_lmat = (int32_t *) &dct_tf8x8_p16;

    matrix::multiply_8x8(dct_lmat, A, tmp);
    for (int i = 0; i < 0x40; i++) {
        DESCALE(tmp[i]);
    }

    int32_t *dct_lmat_trans = new int32_t[0x40];

    matrix::transpose_s<int32_t>(dct_lmat, 8, 8, dct_lmat_trans);
    matrix::multiply_8x8(tmp, dct_lmat_trans, res);

    for (int i = 0; i < 0x40; i++) {
        DESCALE(res[i]);
    }

    free(dct_lmat_trans);
    free(tmp);
}

void dct::apply_idct8x8_p16(int32_t *A, int32_t *res) {
    // res = transpose(P) * A * P
    int32_t *tmp = new int32_t[0x40];

    int32_t *dct_lmat = (int32_t *) &dct_tf8x8_p16;

    int32_t *dct_lmat_trans = new int32_t[0x40];
    matrix::transpose_s<int32_t>(dct_lmat, 8, 8, dct_lmat_trans);

    matrix::multiply_8x8(dct_lmat_trans, A, tmp);
    for (int i = 0; i < 0x40; i++) {
        DESCALE(tmp[i]);
    }


    matrix::multiply_8x8(tmp, dct_lmat, res);

    for (int i = 0; i < 0x40; i++) {
        DESCALE(res[i]);
    }

    free(dct_lmat_trans);
    free(tmp);
}

void dct::apply_dct8x8_pd(double *A, double *res) {
    auto *tmp = new double[0x40];

    auto *dct_lmat = (double *) &dct_tf8x8_pd;

    matrix::multiply_s<double>(dct_lmat, 8, 8, A, 8, tmp);

    auto *dct_lmat_trans = new double[0x40];

    matrix::transpose_s<double>(dct_lmat, 8, 8, dct_lmat_trans);
    matrix::multiply_s<double>(tmp, 8, 8, dct_lmat_trans, 8, res);

    free(dct_lmat_trans);
    free(tmp);
}

void dct::apply_idct8x8_pd(double *A, double *res) {
    auto *tmp = new double[0x40];

    auto *dct_lmat = (double *) &dct_tf8x8_pd;

    auto *dct_lmat_trans = new double[0x40];
    matrix::transpose_s<double>(dct_lmat, 8, 8, dct_lmat_trans);

    matrix::multiply_s<double>(dct_lmat_trans, 8, 8, A, 8, tmp);

    matrix::multiply_s<double>(tmp, 8, 8, dct_lmat, 8, res);

    free(dct_lmat_trans);
    free(tmp);
}

void dct::apply_dct8l_pd(double *A, double *res) {
    auto *dct_lmat = (double *) &dct_tf8x8_pd;

    matrix::multiply_s<double>(dct_lmat, 8, 8, A, 1, res);
}

void dct::apply_idct81_pd(double *A, double *res) {
    auto *dct_lmat = (double *) &dct_tf8x8_pd;

    auto *dct_lmat_trans = new double[0x40];
    matrix::transpose_s<double>(dct_lmat, 8, 8, dct_lmat_trans);

    matrix::multiply_s<double>(dct_lmat_trans, 8, 8, A, 1, res);

    free(dct_lmat_trans);
}
