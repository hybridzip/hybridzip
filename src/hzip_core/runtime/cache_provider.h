#ifndef HZIP_CORE_RUNTIME_CACHE_PROVIDER_H
#define HZIP_CORE_RUNTIME_CACHE_PROVIDER_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <string>
#include <rainman/cache.h>

namespace hzruntime {
    class CacheProvider {
    private:
        static uint64_t _rainman_cache_count;
        static uint64_t _rainman_cache_index;
        static std::vector<rainman::cache> _rainman_caches;
        static std::vector<std::mutex> _rainman_cache_mutexes;
        static std::mutex _rainman_cache_mutex;

    public:
        static void init_cache(const std::string &filename, uint64_t count, uint64_t page_size);

        static std::pair<rainman::cache &, std::mutex &> get_cache();
    };
}

#endif
