project(Google_tests)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(hybridzip_test
        hzip/burrows_wheeler_transform.cpp
        hzip/move_to_front_transform.cpp
        hzip/victini_codec.cpp
        hzip/archive.cpp
        hzip/zigzag_transform.cpp)
target_link_libraries(hybridzip_test gtest gtest_main hzip_core hzip_storage hzip_network)

