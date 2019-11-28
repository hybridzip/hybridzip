cmake_minimum_required(VERSION 3.15)

project(hybridzip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

#include_directories("/usr/local/cuda-10.1/include")

add_library(libhzip STATIC
        #hzip/cuda/gpu_info.h
        #hzip/cuda/gpu_info.cu
        #hzip/cuda/entropy-coders/cuda_fgk.h
        #hzip/cuda/entropy-coders/cuda_fgk.cu
		hzip/bitio/bitio_buffer.cpp
        hzip/bitio/bitio.h
        hzip/bitio/bitio.cpp
		hzip/core/icweb/spawn.cpp
		hzip/core/icweb/icweb.h
		hzip/core/icweb/ctx_tracker.cpp
        hzip/core/entropy-coders/fgk.h
        hzip/core/entropy-coders/fgk.cpp
		hzip/other/platform.h
        hzip/other/constants.h
		hzip/core/entropy-coders/hzrans64.h
		)

target_compile_features(libhzip PUBLIC cxx_std_14)

add_executable(hybridzip hzip/driver/main.cpp)

set_property(TARGET hybridzip PROPERTY CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries(hybridzip PRIVATE libhzip)

