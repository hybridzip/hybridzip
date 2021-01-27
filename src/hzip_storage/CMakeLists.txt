cmake_minimum_required(VERSION 3.16)

add_library(hzip_storage STATIC
        archive/archive.h
        archive/archive_trie.h
        archive/archive.cpp
        errors/archive.h
        )

target_compile_features(hzip_storage PUBLIC cxx_std_20)

target_link_libraries(hzip_storage PUBLIC hzip_core)
target_link_libraries(hzip_storage PUBLIC bitio loguru rainman)
target_link_libraries(hzip_storage PUBLIC crypto)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip_storage DEBUG)
endif ()