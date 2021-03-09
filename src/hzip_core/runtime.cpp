#include "runtime.h"

uint64_t Runtime::_rainman_cache_index = 0;
uint64_t Runtime::_rainman_cache_count = 0;

std::vector<rainman::cache> Runtime::_rainman_caches;
std::vector<std::mutex> Runtime::_rainman_cache_mutexes;

std::mutex Runtime::_rainman_cache_mutex;

void Runtime::init_cache(const std::string &filename, uint64_t count, uint64_t page_size) {
    _rainman_cache_mutexes = std::vector<std::mutex>(count);
    _rainman_caches.clear();

    for (uint64_t index = 0; index < count; index++) {
        _rainman_caches.emplace_back(filename + ".rain." + std::to_string(index), page_size);
    }

    _rainman_cache_count = count;
}

std::pair<rainman::cache&, std::mutex &> Runtime::get_cache() {
    std::scoped_lock<std::mutex> lock(_rainman_cache_mutex);

    auto pair = std::pair<rainman::cache&, std::mutex &>(
            _rainman_caches[_rainman_cache_index],
            _rainman_cache_mutexes[_rainman_cache_index]
    );

    _rainman_cache_index = (_rainman_cache_index + 1) % _rainman_cache_count;
    return pair;
}
