cmake_minimum_required(VERSION 3.16)

project(loguru)

add_library(loguru loguru.hpp loguru.cpp)
target_link_libraries(loguru PUBLIC dl)