#ifndef HYBRIDZIP_AVX2UTILS_H
#define HYBRIDZIP_AVX2UTILS_H

#include <immintrin.h>
#include <cstdint>
#include "../../other/platform.h"


//vector = vector * scalar
HZIP_FORCED_INLINE __m256i avx_mul1plus_256i32(int32_t *vector, int32_t scalar) {
    __m256i sc = _mm256_set1_epi32(scalar);
    __m256i vec = _mm256_setr_epi32(vector[0], vector[1], vector[2], vector[3], vector[4], vector[5], vector[6],
                                    vector[7]);
    __m256i vecplus = _mm256_add_epi32(vec, _mm256_set1_epi32(1));
    return _mm256_mullo_epi32(vecplus, sc);
}

#endif
