cmake_minimum_required(VERSION 3.12)

set(CUDAHOSTCXX "F:/Visual Studio Program Files/Microsoft Visual Studio/2017/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64")
#set(CUDA_FLAGS "--use-local-env --cl-version 2017")
set(CMAKE_CUDA_COMPILER_FORCED TRUE)
project(hybridzip LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 14)

include_directories("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/include")


add_library(libhzip STATIC
        hzip/cuda/gpu_info.h
        hzip/cuda/gpu_info.cu
        hzip/cuda/entropy-coders/cuda_fgk.h
        hzip/cuda/entropy-coders/cuda_fgk.cu
        hzip/bitio/bitio_buffer.h
        hzip/bitio/bitio_buffer.cpp
        hzip/bitio/bitio.h
        hzip/bitio/bitio.cpp
        hzip/core/entropy-coders/fgk.h
        hzip/core/entropy-coders/fgk.cpp
		hzip/other/platform.h
        hzip/other/constants.h
        )

target_compile_features(libhzip PUBLIC cxx_std_14)

add_executable(hybridzip hzip/driver/main.cpp hzip/core/icweb/instance.h hzip/core/icweb/spawn.h hzip/core/icweb/tracker.h hzip/core/icweb/spawn.cpp)

set_property(TARGET hybridzip
        PROPERTY CUDA_SEPARABLE_COMPILATION ON)
target_link_libraries(hybridzip PRIVATE libhzip)

