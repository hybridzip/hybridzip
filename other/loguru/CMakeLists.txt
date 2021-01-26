cmake_minimum_required(VERSION 3.16)

project(loguru)

add_library(loguru src/loguru.cpp)
target_include_directories(loguru
        PUBLIC $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

target_link_libraries(loguru PUBLIC dl)
