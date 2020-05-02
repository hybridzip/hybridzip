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
        src/hzip/utils/avx2utils.h
        src/hzip/core/entropy/hzrans/hzrans64avx.h
        src/hzip/core/entropy/hzrans/hzrans.h
        src/hzip/core/blob/hzblob.h
        src/hzip/core/entropy/hzrans/hzrmthread.h
        src/hzip/utils/unary.h
        src/hzip/core/blob/hzblobpack.h
        src/hzip/utils/distribution.h
        src/hzip/utils/boost_utils.h
        src/hzip/core/entropy/hzrans/hzrbin.h
        src/hzip/archiver/hzarchiver.h
        src/hzip/archiver/hzdirtree.h
        src/hzip/utils/errors.h
        src/hzip/utils/light_stack.h
        src/hzip/utils/fast_math.h
        src/hzip/core/preprocessor/burrows_wheeler_transform.h
        src/hzip/core/models/single_order_context_model.h
        src/hzip/core/preprocessor/move_to_front_transform.h
        src/hzip/core/compressors/dark_ninja_compressor.h src/hzip/core/preprocessor/png_codec.h
        src/hzip/core/models/pivoted_gaussian_model.h
        src/hzip/core/compressors/white_rose_compressor.h
        src/hzip/core/preprocessor/delta_transform.h
        src/hzip/core/preprocessor/transforms.h
        src/hzip/core/models/models.h
        src/hzip/core/compressors/compressors.h
        src/hzip/core/preprocessor/types.h)

target_compile_features(hzip PUBLIC cxx_std_20)
target_link_libraries(hzip PUBLIC bitio yajl)

add_executable(hybridzip src/main.cpp)

target_link_libraries(hybridzip PRIVATE hzip pthread ${Boost_FILESYSTEM_LIBRARY} png turbojpeg)

if (${DISABLE_PROFILER})
    # Do nothing
else ()
    message("Linking to profiler ...")
    target_link_libraries(hybridzip PRIVATE profiler)
endif ()

