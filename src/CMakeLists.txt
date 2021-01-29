add_subdirectory(hzip_core)
add_subdirectory(hzip_network)
add_subdirectory(hzip_storage)

add_executable(hybridzip main.cpp terminal.h)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hybridzip DEBUG)
endif ()

target_link_libraries(hybridzip PRIVATE hzip_core hzip_network hzip_storage)
target_link_libraries(hybridzip PRIVATE pthread)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(hybridzip PRIVATE profiler)
    message(STATUS "linked to profiler")
endif ()