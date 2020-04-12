cmake_minimum_required(VERSION 3.16)
project(yajl)

add_library(yajl STATIC
        yajl.h)

target_compile_features(yajl PUBLIC cxx_std_20)
set_target_properties(yajl PROPERTIES LINKER_LANGUAGE CXX)
