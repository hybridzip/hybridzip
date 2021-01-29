cmake_minimum_required(VERSION 3.16)

add_library(hzip_core STATIC
        utils/platform.h
        core/kernel/hzrans/hzrans64.h
        core/kernel/hzrans/hzrans.h
        core/blob/blob.h
        utils/distribution.h
        utils/fsutils.h
        core/kernel/hzrans/hzrans64_codec.h
        core/preprocessor/png_bundle.h
        core/preprocessor/transforms.h
        core/models/models.h
        core/compressors/compressors.h
        core/preprocessor/jpeg_codec.h
        utils/utils.h
        core/models/first_order_context.cpp
        core/preprocessor/delta.cpp
        core/compressors/victini.cpp
        core/compressors/victini.h
        core/kernel/hzrans/hzrans64_codec.cpp
        core/kernel/hzrans/hzrans64.cpp
        core/models/first_order_context.h
        core/preprocessor/burrows_wheeler.h
        core/preprocessor/delta.h
        core/preprocessor/move_to_front.h
        utils/stack.h
        processor/job.h
        core/compressors/compressor_enums.h
        processor/processor.h
        processor/processor.cpp
        core/compressors/compressor_base.h
        utils/validation.h
        errors/utils.h
        errors/processor.h
        core/preprocessor/zigzag.h
        core/models/paeth.h
        core/preprocessor/color.h
        errors/transform.h
        core/compressors/sharingan.h
        core/compressors/sharingan.cpp
        errors/compression.h
        core/compressors/uncompressed.h)

target_compile_features(hzip_core PUBLIC cxx_std_20)
target_link_libraries(hzip_core PUBLIC bitio loguru rainman)

target_link_libraries(hzip_core PUBLIC crypto png)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip_core DEBUG)
endif ()