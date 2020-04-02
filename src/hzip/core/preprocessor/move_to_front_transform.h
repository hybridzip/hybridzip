#ifndef HYBRIDZIP_MOVE_TO_FRONT_TRANSFORM_H
#define HYBRIDZIP_MOVE_TO_FRONT_TRANSFORM_H

#include <cstdint>
#include <vector>

class MoveToFrontTransformer {
private:
    std::vector<uint64_t> lru_cache;
    int *data;
    uint64_t length;
    int alphabet_size;
public:
    MoveToFrontTransformer(int *data, int alphabet_size, uint64_t length) {
        this->data = data;
        this->length = length;
        this->alphabet_size = alphabet_size;
    }

    void transform() {
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

    void invert() {
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
};

#endif
