add_subdirectory(hzip_core)
add_subdirectory(hzip_codec)
add_subdirectory(hzip_storage)
add_subdirectory(hzip_network)

add_executable(hybridzip main.cpp terminal.h)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hybridzip DEBUG)
endif ()

target_link_libraries(hybridzip PRIVATE hzip_network)
target_link_libraries(hybridzip PRIVATE pthread)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(hybridzip PRIVATE profiler)
    message(STATUS "linked to profiler")
endif ()

if (${HZIP_ENABLE_OPENCL})
    target_compile_definitions(hybridzip PRIVATE HZIP_ENABLE_OPENCL)
endif ()