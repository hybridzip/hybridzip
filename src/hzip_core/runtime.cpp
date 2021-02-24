#include "runtime.h"

uint64_t HZRuntime::_rainman_cache_index = 0;
uint64_t HZRuntime::_rainman_cache_count = 0;

std::vector<rainman::cache> HZRuntime::_rainman_caches;
std::vector<std::mutex> HZRuntime::_rainman_cache_mutexes;

std::mutex HZRuntime::_rainman_cache_mutex;
std::mutex HZRuntime::opencl_mutex;

void HZRuntime::init_cache(const std::string &filename, uint64_t count, uint64_t page_size) {
    for (uint64_t index = 0; index < count; index++) {
        _rainman_caches.emplace_back(filename + ".rain." + std::to_string(index), page_size);
        _rainman_cache_mutexes.emplace_back();
    }
}

std::pair<rainman::cache&, std::mutex &> HZRuntime::get_cache() {
    std::scoped_lock<std::mutex> lock(_rainman_cache_mutex);

    auto pair = std::pair<rainman::cache&, std::mutex &>(
            _rainman_caches[_rainman_cache_index],
            _rainman_cache_mutexes[_rainman_cache_index]
    );

    _rainman_cache_index = (_rainman_cache_index + 1) % _rainman_cache_count;
    return pair;
}
