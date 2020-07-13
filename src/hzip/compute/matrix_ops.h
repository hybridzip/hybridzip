#ifndef HYBRIDZIP_MATRIX_OPS_H
#define HYBRIDZIP_MATRIX_OPS_H

#include "vector_utils.h"
#include <cstdint>
#include <immintrin.h>

namespace hzcompute::matrix {
    // slow matmul functions:

    template <typename elem_type>
    inline void multiply_s(elem_type *A, uint32_t A_rows, uint32_t A_cols, elem_type *B, uint32_t B_cols, elem_type *C) {
        uint32_t B_rows = A_cols;

        uint32_t ic = 0;
        for (int i = 0, rs = 0; i < A_rows; i++, rs += A_cols) {
            for (int j = 0; j < B_cols; j++) {
                C[ic] = 0;
                for (int k = 0, rsb = 0; k < B_rows; k++, rsb += B_cols) {
                    C[ic] += A[rs + k] * B[rsb + j];
                }
                ic++;
            }
        }
    }

    template <typename elem_type>
    inline void transpose_s(elem_type *A, uint32_t n_rows, uint32_t n_cols, elem_type *res) {
        for (int i = 0, ta = 0; i < n_rows; i++, ta += n_cols) {
            for(int j = 0, tb = 0; j < n_cols; j++, tb += n_rows) {
                res[tb + i] = A[ta + j];
            }
        }
    }

    inline void multiply(int32_t *A, uint32_t A_rows, uint32_t A_cols, int32_t *B, uint32_t B_cols, int32_t *C) {
        uint32_t B_rows = A_cols;

#ifdef HZ_USE_AVX2
        uint32_t rsb_max = B_rows * B_cols;
        int32_t *B_trans = new int32_t[rsb_max];
        transpose_s<int32_t>(B, B_rows, B_cols, B_trans);

        uint32_t n_vecs = A_cols >> 0x3;
        uint8_t residue = A_cols & 0x7;

        uint32_t ic = 0;
        for (int i = 0, rsa = 0; i < A_rows; i++, rsa += A_cols) {
            for (int j = 0, rsb = 0; j < B_cols; j++, rsb += B_rows) {
                C[ic] = 0;
                for (int k = 0, rshv = 0; k < n_vecs; k++, rshv += 8) {
                    int32_t *va = A + rsa + rshv;
                    int32_t *vb = B_trans + rsb + rshv;

                    __m256i v_row = _mm256_set_epi32(va[0], va[1], va[2], va[3],
                                                     va[4], va[5], va[6], va[7]);
                    __m256i v_col = _mm256_set_epi32(vb[0], vb[1], vb[2], vb[3],
                                                     vb[4], vb[5], vb[6], vb[7]);
                    __m256i v_res = _mm256_mullo_epi32(v_row, v_col);
                    C[ic] += utils::hsum_i32_mm256i(v_res);
                }

                int rshv = n_vecs << 3;
                for (int k = 0; k < residue; k++) {
                    C[ic] += A[rsa + rshv + k] * B_trans[rsb + rshv + k];
                }
                ic++;
            }
        }

        free(B_trans);
#else
        uint32_t ic = 0;
        for (int i = 0, rs = 0; i < A_rows; i++, rs += A_cols) {
            for (int j = 0; j < B_cols; j++) {
                C[ic] = 0;
                for (int k = 0, rsb = 0; k < B_rows; k++, rsb += B_cols) {
                    C[ic] += A[rs + k] * B[rsb + j];
                }
                ic++;
            }
        }
#endif

    }


    inline void multiply_8x8(int32_t *A, int32_t *B, int32_t *C) {
#ifdef HZ_USE_AVX2
        __m256i colvec[8];
        for (int j = 0; j < 8; j++) {
            colvec[j] = _mm256_set_epi32(B[0x38 + j], B[0x30 + j], B[0x28 + j], B[0x20 + j],
                                         B[0x18 + j], B[0x10 + j], B[0x8 + j], B[j]);
        }


        for (int i = 0; i < 8; i++) {
            uint8_t rs = i << 3;
            int32_t *va = (A + rs);
            __m256i vec_a = _mm256_set_epi32(va[7], va[6], va[5], va[4],
                                             va[3], va[2], va[1], va[0]);


            for (int j = 0; j < 8; j++) {
                __m256i vec_b = colvec[j];
                __m256i vec_c = _mm256_mullo_epi32(vec_a, vec_b);
                C[rs + j] = utils::hsum_i32_mm256i(vec_c);
            }
        }


#else
        // use slow matrix multiplication.

        for (int i = 0; i < 8; i++) {
            uint32_t rs = i << 3;
            for (int j = 0; j < 8; j++) {
                C[rs + j] = 0;
                for (int k = 0; k < 8; k++) {
                    C[rs + j] += A[rs + k] * B[(k << 3) + j];
                }
            }
        }

#endif
    }

    inline void multiply_8x8(double *A, double *B, double *C) {
        
    }
}

#endif
