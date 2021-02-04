cmake_minimum_required(VERSION 3.16)

add_library(hzip_core STATIC
        utils/platform.h
        kernel/hzrans/hzrans64.h
        kernel/hzrans/hzrans.h
        blob/blob.h
        utils/distribution.h
        utils/fsutils.h
        kernel/hzrans/hzrans64_codec.h
        preprocessor/png_bundle.h
        preprocessor/transforms.h
        models/models.h
        preprocessor/jpeg_codec.h
        utils/utils.h
        models/first_order_context.cpp
        preprocessor/delta.cpp
        kernel/hzrans/hzrans64_codec.cpp
        kernel/hzrans/hzrans64.cpp
        models/first_order_context.h
        preprocessor/burrows_wheeler.h
        preprocessor/delta.h
        preprocessor/move_to_front.h
        utils/stack.h
        utils/validation.h
        errors/utils.h
        preprocessor/zigzag.h
        models/paeth.h
        preprocessor/color.h
        errors/transform.h
        )

target_compile_features(hzip_core PUBLIC cxx_std_20)
target_link_libraries(hzip_core PUBLIC bitio loguru rainman)

target_link_libraries(hzip_core PUBLIC crypto png)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip_core DEBUG)
endif ()