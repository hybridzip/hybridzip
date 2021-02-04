cmake_minimum_required(VERSION 3.16)

add_library(hzip_sharingan STATIC sharingan.h sharingan.cpp)

target_link_libraries(hzip_sharingan PRIVATE hzip_core)