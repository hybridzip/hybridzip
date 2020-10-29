cmake_minimum_required(VERSION 3.17)
project(tensorflow)

set(CMAKE_CXX_STANDARD 20)

link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/linux-x86_64")
add_library(tensorflow-lite STATIC IMPORTED GLOBAL)
set_target_properties(tensorflow-lite PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/linux-x86_64/libtensorflow-lite.a")