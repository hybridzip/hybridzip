project(Google_tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(gtest_run hzip/memory.cpp hzip/burrows_wheeler_transform.cpp hzip/move_to_front_transform.cpp hzip/victini_codec.cpp hzip/archive.cpp)
target_link_libraries(gtest_run gtest gtest_main hzip)

