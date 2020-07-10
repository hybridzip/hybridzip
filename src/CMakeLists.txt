add_subdirectory(hzip)

add_executable(hybridzip main.cpp terminal.h)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hybridzip DEBUG)
endif ()

target_link_libraries(hybridzip PRIVATE hzip pthread png jpeg)

if (${ENABLE_PROFILING})
    target_link_libraries(hybridzip PRIVATE profiler)
    message(STATUS "linked to profiler")
endif ()