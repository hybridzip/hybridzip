cmake_minimum_required(VERSION 3.16)

add_library(hzip_uncompressed STATIC uncompressed.h uncompressed.cpp)

target_link_libraries(hzip_uncompressed PRIVATE hzip_core)