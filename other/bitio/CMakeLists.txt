cmake_minimum_required(VERSION 3.16)
project(bitio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(bitio STATIC
        bitio.h
        bitio.cpp
        bitio_buffer.cpp
        bitio_byte_dumper.cpp
        )

target_compile_features(bitio PUBLIC cxx_std_20)
set_target_properties(bitio PROPERTIES LINKER_LANGUAGE CXX)