#include "config.h"
#include "runtime.h"
#include <cstdlib>
#include <string>
#include <hzip_core/opencl/cl_helper.h>

uint64_t Config::api_threads = 1;
uint64_t Config::api_port = 1729;
uint32_t Config::api_timeout = 100;
std::string Config::api_key = "hybridzip";

uint64_t Config::processor_threads = 1;

uint64_t Config::host_max_memory = 1073741824;

bool Config::opencl_support_enabled = false;
uint64_t Config::opencl_kernels = 32;
std::string Config::opencl_preferred_device = "";

void Config::configure() {
    const char *_cache_file = std::getenv("HZIP_CACHE_FILE");
    const char *_cache_page_size = std::getenv("HZIP_CACHE_PAGE_SIZE");
    const char *_cache_count = std::getenv("HZIP_CACHE_COUNT");
    const char *_api_threads = std::getenv("HZIP_API_THREADS");
    const char *_api_port = std::getenv("HZIP_API_PORT");
    const char *_api_timeout = std::getenv("HZIP_API_TIMEOUT");
    const char *_api_key = std::getenv("HZIP_API_KEY");
    const char *_processor_threads = std::getenv("HZIP_PROCESSOR_THREADS");
    const char *_host_max_memory = std::getenv("HZIP_MAX_MEM_USAGE");

    if (_cache_file != nullptr) {
        uint64_t page_size = 131072;
        uint64_t cache_count = 4;

        if (_cache_page_size != nullptr) {
            page_size = std::stoull(_cache_page_size);
        }

        if (_cache_count != nullptr) {
            cache_count = std::stoull(_cache_count);
        }

        HZRuntime::init_cache(_cache_file, cache_count, page_size);
    } else {
        HZRuntime::init_cache("hzip", 4, 131072);
    }

    if (_api_threads != nullptr) {
        Config::api_threads = std::stoull(_api_threads);
    }

    if (_api_port != nullptr) {
        Config::api_port = std::stoull(_api_port);
    }

    if (_api_timeout != nullptr) {
        Config::api_timeout = std::stoull(_api_timeout);
    }

    if (_api_key != nullptr) {
        Config::api_key = _api_key;
    }

    if (_processor_threads != nullptr) {
        Config::processor_threads = std::stoull(_processor_threads);
    }

    if (_host_max_memory != nullptr) {
        Config::host_max_memory = std::stoull(_host_max_memory);
    }

#ifdef HZIP_ENABLE_OPENCL
    hzopencl::DeviceProvider::load_devices();

    if (!hzopencl::DeviceProvider::empty()) {
        opencl_support_enabled = true;
    }

    const char *_opencl_kernels = std::getenv("HZIP_OPENCL_KERNELS");
    const char *_opencl_preferred_device = std::getenv("HZIP_OPENCL_PREFERRED_DEVICE");

    if (_opencl_kernels != nullptr) {
        Config::opencl_kernels = std::stoull(_opencl_kernels);
    }

    if (_opencl_preferred_device != nullptr) {
        Config::opencl_preferred_device = _opencl_preferred_device;
        hzopencl::DeviceProvider::set_preferred_device(Config::opencl_preferred_device);
    }
#endif
}
