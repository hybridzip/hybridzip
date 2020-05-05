#include "transforms.h"

hz_trans::mtf_transformer::mtf_transformer(int *data, int alphabet_size, uint64_t length) {
    this->data = data;
    this->length = length;
    this->alphabet_size = alphabet_size;
}

void hz_trans::mtf_transformer::transform() {
    lru_cache.clear();
    for (int i = 0; i < alphabet_size; i++) {
        lru_cache.push_back(i);
    }
    for (uint64_t i = 0; i < length; i++) {
        uint64_t symbol = data[i];
        for (int j = 0; j < alphabet_size; j++) {
            if (symbol == lru_cache[j]) {
                data[i] = j;
                lru_cache.erase(lru_cache.begin() + j);
                lru_cache.insert(lru_cache.begin(), symbol);
                break;
            }
        }
    }
}

void hz_trans::mtf_transformer::invert() {
    lru_cache.clear();
    for (int i = 0; i < alphabet_size; i++) {
        lru_cache.push_back(i);
    }
    for (uint64_t i = 0; i < length; i++) {
        auto chri = data[i];
        data[i] = lru_cache[chri];
        lru_cache.erase(lru_cache.begin() + chri);
        lru_cache.insert(lru_cache.begin(), data[i]);
    }
}

