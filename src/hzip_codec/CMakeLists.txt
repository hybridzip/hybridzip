cmake_minimum_required(VERSION 3.16)

# Add codecs
set(HZIP_CODEC_TARGETS)
add_subdirectory(uncompressed)
add_subdirectory(victini)
add_subdirectory(sharingan)

add_library(hzip_codec STATIC
        compressors.h
        compressor_enums.h
        compressor_base.h
        errors/codec.h
        errors/processor.h
        processor/job.h
        processor/processor.h
        processor/processor.cpp
        codec_provider.h
        codec_provider.cpp
        opencl/cl_helper.h
        opencl/cl_helper.cpp
        errors/opencl.h
        executor.h
        )

target_link_libraries(hzip_codec PUBLIC
        hzip_uncompressed
        hzip_victini
        hzip_sharingan
        )

target_link_libraries(hzip_codec PUBLIC hzip_core)

if (${HZIP_ENABLE_OPENCL})
    target_link_libraries(hzip_codec PUBLIC OpenCL)
    target_compile_definitions(hzip_codec PRIVATE HZIP_ENABLE_OPENCL)
endif ()