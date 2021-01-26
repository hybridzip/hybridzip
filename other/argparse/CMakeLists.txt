cmake_minimum_required(VERSION 3.16)

project(argparse LANGUAGES CXX)

add_library(argparse include/argparse/argparse.h)
set_target_properties(argparse PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(argparse
        PUBLIC $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )