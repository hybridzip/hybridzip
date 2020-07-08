cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CheckSIMDSupport)

project(hybridzip LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

include_directories("src")

add_subdirectory(other)
include_directories("other")

add_library(hzip STATIC
        src/hzip/utils/platform.h
        src/hzip/core/entropy/hzrans/hzrans64.h
        src/hzip/core/entropy/hzrans/hzrans.h
        src/hzip/core/blob/hzblob.h
        src/hzip/core/entropy/hzrans/hzmthread.h
        src/hzip/core/blob/hzblobpack.h
        src/hzip/utils/distribution.h
        src/hzip/utils/fsutils.h
        src/hzip/core/entropy/hzrans/hzbin.h
        src/hzip/archiver/hzarchiver.h
        src/hzip/archiver/hzdirtree.h
        src/hzip/utils/errors.h
        src/hzip/core/preprocessor/png_codec.h
        src/hzip/core/preprocessor/transforms.h
        src/hzip/core/models/models.h
        src/hzip/core/compressors/compressors.h
        src/hzip/core/preprocessor/types.h
        src/hzip/core/preprocessor/jpeg_codec.h
        src/hzip/utils/utils.h
        src/hzip/core/models/first_order_context_model.cpp
        src/hzip/core/preprocessor/bw_transformer.cpp
        src/hzip/core/preprocessor/delta_transformer.cpp
        src/hzip/core/preprocessor/mtf_transformer.cpp
        src/hzip/core/compressors/white_rose.cpp
        src/hzip/core/compressors/white_rose.h
        src/hzip/core/entropy/hzrans/hzmthread.cpp
        src/hzip/core/entropy/hzrans/hzbin.cpp
        src/hzip/core/entropy/hzrans/hzrans64.cpp
        src/hzip/core/models/first_order_context_model.h
        src/hzip/core/blob/hzblobpack.cpp
        src/hzip/core/preprocessor/bw_transformer.h
        src/hzip/core/preprocessor/delta_transformer.h
        src/hzip/core/preprocessor/mtf_transformer.h
        src/hzip/compute/matrix_ops.h
        src/hzip/compute/vector_utils.h
        src/hzip/compute/dct.h
        src/hzip/compute/dct.cpp src/hzip/memory/memmgr.h src/hzip/memory/memmap.h src/hzip/memory/memmgr.cpp src/hzip/memory/memmap.cpp src/hzip/memory/mem_interface.h src/hzip/errors/memory.h)

target_compile_features(hzip PUBLIC cxx_std_20)
target_link_libraries(hzip PUBLIC bitio loguru)

# Find Intel Math Kernel Libs
find_package(MKL)

if (${MKL_FOUND})
    include_directories(${MKL_INCLUDE_DIRS})
    target_link_libraries(hzip PUBLIC ${MKL_LIBRARIES})
endif ()

add_compile_definitions(hzip HZ_USE_INTEL_MKL)
check_simd_support(ARCH_FLAGS)

if (ENABLE_AVX512)
    add_compile_definitions(hzip HZ_USE_AVX512)
    message(STATUS "Enabled AVX512 support")
elseif (ENABLE_AVX2)
    add_compile_definitions(hzip HZ_USE_AVX2)
    message(STATUS "Enabled AVX2 support")
elseif (ENABLE_SSE2)
    add_compile_definitions(hzip HZ_USE_SSE2)
    message(STATUS "Enabled SSE2 support")
endif ()

target_compile_options(hzip PUBLIC ${ARCH_FLAGS})

add_executable(hybridzip src/main.cpp)
target_link_libraries(hybridzip PRIVATE hzip pthread png jpeg)

if (${ENABLE_PROFILING})
    target_link_libraries(hybridzip PRIVATE profiler)
    message(STATUS "Linked to profiler")
endif ()

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip DEBUG)
endif ()


enable_testing()

add_subdirectory(tests)