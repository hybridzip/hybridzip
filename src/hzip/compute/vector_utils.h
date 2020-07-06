#ifndef HYBRIDZIP_VECTOR_UTILS_H
#define HYBRIDZIP_VECTOR_UTILS_H

#include <immintrin.h>

#if defined(HZ_USE_AVX2) || defined(HZ_USE_AVX512)

namespace hzcompute::utils {
    inline double hsum_double_mm256d(__m256d v) {
        __m128d vlow = _mm256_castpd256_pd128(v);
        __m128d vhigh = _mm256_extractf128_pd(v, 1);
        vlow = _mm_add_pd(vlow, vhigh);

        __m128d high64 = _mm_unpackhi_pd(vlow, vlow);
        return _mm_cvtsd_f64(_mm_add_sd(vlow, high64));
    }

    inline int32_t hsum_i32_mm256i(__m256i v) {
        __m128i vlow = _mm256_castsi256_si128(v);
        __m128i vhigh = _mm256_extractf128_si256(v, 1);

        vlow = _mm_add_epi32(vlow, vhigh);
        vhigh = _mm_unpackhi_epi64(vlow, vlow);
        vlow = _mm_add_epi32(vlow, vhigh);

        int64_t s = ((int64_t *) &vlow)[0];
        return (s & 0xFFFFFFFF) + (s >> 0x20);
    }
}

#endif


#endif
