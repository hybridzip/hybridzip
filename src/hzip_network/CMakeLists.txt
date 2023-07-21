cmake_minimum_required(VERSION 3.16)

add_library(hzip_network STATIC
        api/api.h
        api/api_enums.h
        api/api.cpp
        api/handlers/query.h
        api/handlers/query.cpp
        api/handlers/socket_class.h
        api/handlers/socket_class.cpp
        api/handlers/streamer.h
        api/handlers/streamer.cpp
        api/providers/archive_provider.h
        api/providers/archive_provider.cpp
        errors/api.h
        )

set_target_properties(hzip_network PROPERTIES CXX_STANDARD 20)

target_link_libraries(hzip_network PUBLIC hzip_core hzip_storage hzip_codec)
target_link_libraries(hzip_network PUBLIC loguru rainman)
target_link_libraries(hzip_network PUBLIC crypto png)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(hzip_network DEBUG)
endif ()