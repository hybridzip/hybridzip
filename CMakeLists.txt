cmake_minimum_required(VERSION 3.12)

set(CUDAHOSTCXX "F:/Visual Studio Program Files/Microsoft Visual Studio/2017/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64")
project(hybridzip LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 14)

include_directories("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/include")


add_library(libhzip STATIC
        cuda/gpu_info.h
		cuda/gpu_info.cu
		host/bitio.h
		host/bitio.cpp
		host/ring_buffer.h
		host/ring_buffer.cpp
		host/platform.h
		host/constants.h
        )

target_compile_features(libhzip PUBLIC cxx_std_14)

add_executable(hybridzip main.cpp)

set_property(TARGET hybridzip 
             PROPERTY CUDA_SEPARABLE_COMPILATION ON)
target_link_libraries(hybridzip PRIVATE libhzip)

