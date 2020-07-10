cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CheckSIMDSupport)

project(hybridzip LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

include_directories("src")
include_directories("other")

add_subdirectory(other)
add_subdirectory(src)

enable_testing()

add_subdirectory(tests)