#ifndef HZIP_CORE_RUNTIME_H
#define HZIP_CORE_RUNTIME_H

#include <mutex>

class HZRuntime {
public:
    static std::mutex rainman_cache_mutex;
    static std::mutex opencl_mutex;
};

#endif
