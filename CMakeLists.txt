cmake_minimum_required(VERSION 3.15)

project(hybridzip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories("src")

add_subdirectory(other)
include_directories("other")

find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


add_library(hzip STATIC
        src/hzip/utils/platform.h
        src/hzip/utils/constants.h
        src/hzip/core/entropy/hzrans/hzrans64.h
        src/hzip/core/entropy/hzrans/hzrans.h
        src/hzip/core/blob/hzblob.h
        src/hzip/core/entropy/hzrans/hzmthread.h
        src/hzip/core/blob/hzblobpack.h
        src/hzip/utils/distribution.h
        src/hzip/utils/boost_utils.h
        src/hzip/core/entropy/hzrans/hzbin.h
        src/hzip/archiver/hzarchiver.h
        src/hzip/archiver/hzdirtree.h
        src/hzip/utils/errors.h
        src/hzip/core/preprocessor/png_codec.h
        src/hzip/core/preprocessor/transforms.h
        src/hzip/core/models/models.h
        src/hzip/core/compressors/compressors.h
        src/hzip/core/preprocessor/types.h
        src/hzip/core/preprocessor/jpeg_codec.h
        src/hzip/utils/utils.h
        src/hzip/core/models/pivoted_gaussian_model.cpp
        src/hzip/core/models/first_order_context_model.cpp
        src/hzip/core/preprocessor/bw_transformer.cpp
        src/hzip/core/preprocessor/delta_transformer.cpp
        src/hzip/core/preprocessor/mtf_transformer.cpp
        src/hzip/core/compressors/white_rose.cpp
        src/hzip/core/compressors/white_rose.h
        src/hzip/core/compressors/dark_ninja.h
        src/hzip/core/compressors/dark_ninja.cpp src/hzip/utils/utils.cpp src/hzip/core/entropy/hzrans/hzmthread.cpp src/hzip/core/entropy/hzrans/hzbin.cpp src/hzip/core/entropy/hzrans/hzrans64.cpp src/hzip/core/models/pivoted_gaussian_model.h src/hzip/core/models/first_order_context_model.h src/hzip/core/blob/hzblobpack.cpp)

target_compile_features(hzip PUBLIC cxx_std_20)
target_link_libraries(hzip PUBLIC bitio)

add_executable(hybridzip src/main.cpp)

target_link_libraries(hybridzip PRIVATE hzip pthread ${Boost_FILESYSTEM_LIBRARY} png jpeg)

if (${DISABLE_PROFILER})
    # Do nothing
else ()
    message("Linking to profiler ...")
    target_link_libraries(hybridzip PRIVATE profiler)
endif ()

