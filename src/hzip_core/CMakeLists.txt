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
        preprocessor/color.cpp
        opencl/cl_helper.h
        opencl/cl_helper.cpp
        runtime/executor.h
        errors/opencl.h
        utils/semaphore.h utils/semaphore.cpp runtime/config.h runtime/config.cpp models/paeth.cpp runtime/cache_provider.h runtime/cache_provider.cpp runtime/runtime.h preprocessor/png_bundle.cpp)

target_compile_features(hzip_core PUBLIC cxx_std_20)
target_link_libraries(hzip_core PUBLIC bitio loguru rainman)

target_link_libraries(hzip_core PUBLIC crypto png)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(hzip_core PRIVATE DEBUG)
endif ()

if (${HZIP_ENABLE_OPENCL})
    target_link_libraries(hzip_core PUBLIC OpenCL)
    target_compile_definitions(hzip_core PRIVATE HZIP_ENABLE_OPENCL CL_HPP_ENABLE_EXCEPTIONS)
endif ()