cmake_minimum_required(VERSION 3.16)

add_library(hzip_sharingan STATIC sharingan.h sharingan.cpp)

target_link_libraries(hzip_sharingan PRIVATE hzip_core)

if (${HZIP_ENABLE_OPENCL})
    target_link_libraries(hzip_sharingan PRIVATE OpenCL)
    target_compile_definitions(hzip_sharingan PRIVATE HZIP_ENABLE_OPENCL CL_HPP_ENABLE_EXCEPTIONS)
endif ()