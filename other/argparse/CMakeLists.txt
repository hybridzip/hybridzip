cmake_minimum_required(VERSION 3.16)

project(argparse LANGUAGES CXX)

add_library(argparse argparse.h)
set_target_properties(argparse PROPERTIES LINKER_LANGUAGE CXX)