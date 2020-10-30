cmake_minimum_required(VERSION 3.16)

add_library(hzip STATIC
        utils/platform.h
        core/kernel/hzrans/hzrans64.h
        core/kernel/hzrans/hzrans.h
        core/blob/hzblob.h
        utils/distribution.h
        utils/fsutils.h
        core/kernel/hzrans/hzrans64_codec.h
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
        core/kernel/hzrans/hzrans64_codec.cpp
        core/kernel/hzrans/hzrans64.cpp
        core/models/first_order_context_model.h
        core/preprocessor/bw_transformer.h
        core/preprocessor/delta_transformer.h
        core/preprocessor/mtf_transformer.h
        compute/matrix_ops.h
        compute/vector_utils.h
        compute/dct.h
        compute/dct.cpp
        utils/stack.h
        processor/job.h
        archive/archive.h
        core/compressors/compressor_enums.h
        processor/processor.h
        processor/processor.cpp
        core/compressors/compressor_base.h
        archive/archive.cpp
        errors/archive.h
        utils/validation.h
        errors/utils.h
        api/api.h
        api/api.cpp errors/api.h
        api/handlers/stream.h
        api/handlers/stream.cpp
        api/handlers/socket_class.h
        api/handlers/socket_class.cpp
        errors/processor.h
        api/providers/archive_provider.h
        api/providers/archive_provider.cpp
        api/api_enums.h
        api/handlers/query.h
        api/handlers/query.cpp)

target_compile_features(hzip PUBLIC cxx_std_20)
target_link_libraries(hzip PUBLIC bitio loguru rainman cpp_dotenv tensorflowlite)

target_link_libraries(hzip PUBLIC crypto)

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