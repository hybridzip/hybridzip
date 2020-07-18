project(Google_tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(gtest_run memory.cpp burrows_wheeler_transform.cpp move_to_front_transform.cpp victini_codec.cpp)
target_link_libraries(gtest_run gtest gtest_main hzip)

