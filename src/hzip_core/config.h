#ifndef HZIP_CORE_CONFIG_H
#define HZIP_CORE_CONFIG_H
#include <cstdint>
#include <string>

class Config {
public:
    static uint64_t api_threads;
    static uint64_t api_port;
    static uint32_t api_timeout;
    static std::string api_key;
    static uint64_t processor_threads;
    static uint64_t host_max_memory;
    static uint64_t opencl_kernels;
    static std::string opencl_preferred_device;

    static void configure();
};

#endif
