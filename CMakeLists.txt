cmake_minimum_required(VERSION 3.15)

project(hybridzip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(libhzip STATIC
        #hzip/cuda/gpu_info.h
        #hzip/cuda/gpu_info.cu
        #hzip/cuda/entropy/cuda_fgk.h
        #hzip/cuda/entropy/cuda_fgk.cu
		hzip/bitio/bitio_buffer.cpp
        hzip/bitio/bitio.h
        hzip/bitio/bitio.cpp
		hzip/core/entropy/fgk/fgk.h
		hzip/core/entropy/fgk/fgk.cpp
		hzip/other/platform.h
        hzip/other/constants.h
		hzip/core/entropy/hzrans/hzrans64.h
		hzip/core/entropy/hzrans/hzrbyte.h
		hzip/core/utils/avx2utils.h
		hzip/core/entropy/hzrans/hzrans64avx.h
		hzip/core/entropy/hzrans/hzrans.h
		hzip/core/blob/hzblob.h
		hzip/core/blob/hzmthread.h
		hzip/core/utils/unary.h
		hzip/core/blob/hzblobpack.h
		hzip/bitio/bitio_byte_dumper.cpp
		hzip/core/utils/distribution.h
		hzip/core/blob/hzbatchprocessor.h
		hzip/core/utils/boost_utils.h
		)

target_compile_features(libhzip PUBLIC cxx_std_14)

add_executable(hybridzip hzip/driver/main.cpp)

set_property(TARGET hybridzip PROPERTY CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries(hybridzip PRIVATE libhzip pthread ${Boost_FILESYSTEM_LIBRARY})

