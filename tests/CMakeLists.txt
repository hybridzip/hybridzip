project(Google_tests)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(hybridzip_test
        hzip_core/burrows_wheeler_transform.cpp
        hzip_core/move_to_front_transform.cpp
        hzip_codec/victini_codec.cpp
        hzip_storage/archive.cpp
        hzip_core/zigzag_transform.cpp
        hzip_codec/sharingan_codec.cpp
        hzip_core/color_transform.cpp debug.cpp)

target_link_libraries(hybridzip_test gtest gtest_main hzip_core hzip_storage hzip_network hzip_codec)

if (${HZIP_ENABLE_OPENCL})
    target_compile_definitions(hybridzip_test PRIVATE HZIP_ENABLE_OPENCL)
endif ()

