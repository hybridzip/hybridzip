cmake_minimum_required(VERSION 3.16)

add_library(hzip_victini STATIC victini.h victini.cpp)

target_link_libraries(hzip_victini PRIVATE hzip_core)