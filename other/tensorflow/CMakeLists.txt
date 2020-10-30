cmake_minimum_required(VERSION 3.17)
project(tensorflow)

set(CMAKE_CXX_STANDARD 20)

link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/linux-x86_64")

add_library(tensorflowlite_gpu_delegate SHARED IMPORTED GLOBAL)
set_target_properties(tensorflowlite_gpu_delegate PROPERTIES IMPORTED_LOCATION_DEBUG
        "${CMAKE_CURRENT_LIST_DIR}/lib/linux-x86_64/libtensorflowlite_gpu_delegate.so")

set_target_properties(tensorflowlite_gpu_delegate PROPERTIES IMPORTED_LOCATION_RELEASE
        "/usr/lib/libtensorflowlite_gpu_delegate.so")

add_library(tensorflowlite SHARED IMPORTED GLOBAL)
set_target_properties(tensorflowlite PROPERTIES IMPORTED_LOCATION_DEBUG
        "${CMAKE_CURRENT_LIST_DIR}/lib/linux-x86_64/libtensorflowlite.so")

set_target_properties(tensorflowlite PROPERTIES IMPORTED_LOCATION_RELEASE
        "/usr/lib/libtensorflowlite.so")

target_link_libraries(tensorflowlite INTERFACE tensorflowlite_gpu_delegate GL EGL)
