cmake_minimum_required(VERSION 3.15)

project(hybridzip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
include_directories("src")

find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(hzip STATIC
        #hzip/cuda/gpu_info.h
        #hzip/cuda/gpu_info.cu
        #hzip/cuda/entropy/cuda_fgk.h
        #hzip/cuda/entropy/cuda_fgk.cu
		src/hzip/bitio/bitio_buffer.cpp
		src/hzip/bitio/bitio.h
		src/hzip/bitio/bitio.cpp
		src/hzip/other/platform.h
		src/hzip/other/constants.h
		src/hzip/core/entropy/hzrans/hzrans64.h
        src/hzip/utils/avx2utils.h
		src/hzip/core/entropy/hzrans/hzrans64avx.h
		src/hzip/core/entropy/hzrans/hzrans.h
		src/hzip/core/blob/hzblob.h
		src/hzip/core/blob/hzmthread.h
		src/hzip/utils/unary.h
		src/hzip/core/blob/hzblobpack.h
		src/hzip/bitio/bitio_byte_dumper.cpp
		src/hzip/utils/distribution.h
		src/hzip/core/blob/hzbatchprocessor.h
		src/hzip/utils/boost_utils.h
		src/hzip/core/entropy/hzrans/hzrbin.h
		src/hzip/archiver/hzarchiver.h
		src/hzip/archiver/hzdirtree.h
		src/hzip/log/logger.h
		src/hzip/utils/errors.h
		src/hzip/log/logger.cpp src/hzip/core/models/vtrie.h)

target_compile_features(hzip PUBLIC cxx_std_20)

add_executable(hybridzip src/main.cpp)

target_link_libraries(hybridzip PRIVATE hzip pthread ${Boost_FILESYSTEM_LIBRARY} profiler)

