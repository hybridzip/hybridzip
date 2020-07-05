include(CheckCSourceRuns)

macro(check_simd_support VAR)
    if (NOT (DEFINED VAR))
        set(${VAR} "")
    endif ()


    if (NOT (DEFINED ENABLE_SSE2))
        set(ENABLE_SSE2 OFF)
    endif ()

    if (NOT (DEFINED ENABLE_AVX2))
        set(ENABLE_AVX2 OFF)
    endif ()

    if (NOT (DEFINED ENABLE_AVX512))
        set(ENABLE_AVX512 OFF)
    endif ()

    if (${ENABLE_SSE2})
        if (WIN32)
            set(CMAKE_REQUIRED_FLAGS "/arch:SSE2")
        else ()
            set(CMAKE_REQUIRED_FLAGS "-msse4.2")
        endif ()

        list(APPEND ${VAR} ${CMAKE_REQUIRED_FLAGS})

        check_c_source_runs("#include <immintrin.h>
            int main() {
                __m128i a = _mm_set1_epi8(1);
                (void)_mm_shuffle_epi8(a, a);
            }" SUPPORTS_SSE2)
        unset(CMAKE_REQUIRED_FLAGS)
    endif ()

    if (${ENABLE_SSE2} AND (NOT ("${SUPPORTS_SSE2}" STREQUAL "1")))
        message(FATAL_ERROR "SSE2 support was requested but was not found")
    endif ()

    if (${ENABLE_AVX2})
        if (WIN32)
            set(CMAKE_REQUIRED_FLAGS "/arch:AVX2")
        else ()
            set(CMAKE_REQUIRED_FLAGS "-march=core-avx2")
        endif ()

        list(APPEND ${VAR} ${CMAKE_REQUIRED_FLAGS})

        check_c_source_runs("#include <immintrin.h>
        #if !defined(__AVX2__)
        #error no avx2
        #endif

        int main(){
            __m256i z = _mm256_setzero_si256();
            (void)_mm256_xor_si256(z, z);
        }
        " SUPPORTS_AVX2)
        unset(CMAKE_REQUIRED_FLAGS)
    endif ()

    if (${ENABLE_AVX2} AND (NOT ("${SUPPORTS_AVX2}" STREQUAL "1")))
        message(FATAL_ERROR "AVX2 support was requested but was not found")
    endif ()

    if (${ENABLE_AVX512})
        if (WIN32)
            set(CMAKE_REQUIRED_FLAGS "/arch:AVX512")
        else ()
            set(CMAKE_REQUIRED_FLAGS "-march=skylake-avx512")
        endif ()

        list(APPEND ${VAR} ${CMAKE_REQUIRED_FLAGS})

        check_c_source_runs("#include <immintrin.h>
        #if !defined(__AVX512BW__)
        #error no avx512bw
        #endif
        int main(){
            __m512i z = _mm512_setzero_si512();
            (void)_mm512_abs_epi8(z);
        }" SUPPORTS_AVX512)
        unset(CMAKE_REQUIRED_FLAGS)
    endif ()

    if (${ENABLE_AVX512} AND (NOT ("${SUPPORTS_AVX512}" EQUAL "1")))
        message(FATAL_ERROR "AVX512 support was requested but was not found")
    endif ()

    unset(SUPPORTS_AVX2)
    unset(SUPPORTS_AVX512)
endmacro(check_simd_support)