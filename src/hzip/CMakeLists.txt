cmake_minimum_required(VERSION 3.16)

add_library(hzip STATIC
        utils/platform.h
        core/entropy/hzrans/hzrans64.h
        core/entropy/hzrans/hzrans.h
        core/blob/hzblob.h
        utils/distribution.h
        utils/fsutils.h
        core/entropy/hzrans/hzbin.h
        core/preprocessor/png_codec.h
        core/preprocessor/transforms.h
        core/models/models.h
        core/compressors/compressors.h
        core/preprocessor/types.h
        core/preprocessor/jpeg_codec.h
        utils/utils.h
        core/models/first_order_context_model.cpp
        core/preprocessor/delta_transformer.cpp
        core/compressors/victini.cpp
        core/compressors/victini.h
        core/entropy/hzrans/hzbin.cpp
        core/entropy/hzrans/hzrans64.cpp
        core/models/first_order_context_model.h
        core/preprocessor/bw_transformer.h
        core/preprocessor/delta_transformer.h
        core/preprocessor/mtf_transformer.h
        compute/matrix_ops.h
        compute/vector_utils.h
        compute/dct.h
        compute/dct.cpp
        memory/memmgr.h
        memory/memmap.h
        memory/memmgr.cpp
        memory/memmap.cpp
        memory/mem_interface.h
        errors/memory.h
        utils/stack.h
        processor/dispatcher.h
        processor/job.h
        archive/archive.h
        processor/dispatcher.cpp
        core/compressors/compressor_enums.h
        processor/processor.h
        processor/processor.cpp
        core/compressors/compressor_base.h
        archive/archive.cpp
        errors/archive.h)

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

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip DEBUG)
endif ()