#ifndef HYBRIDZIP_MOVE_TO_FRONT_H
#define HYBRIDZIP_MOVE_TO_FRONT_H

#include <vector>
#include <cstdint>
#include <rainman/rainman.h>

namespace hztrans {

    template<typename itype>
    class MoveToFrontTransformer {
    private:
        std::vector<itype> lru_cache;
        rainman::ptr<itype> data;
        uint64_t alphabet_size{};
    public:
        MoveToFrontTransformer(const rainman::ptr<itype> &data, uint64_t alphabet_size) {
            this->data = data;
            this->alphabet_size = alphabet_size;
        }

        void transform() {
            lru_cache.clear();
            for (uint64_t i = 0; i < alphabet_size; i++) {
                lru_cache.push_back(i);
            }
            for (uint64_t i = 0; i < data.size(); i++) {
                auto symbol = data[i];
                for (uint64_t j = 0; j < alphabet_size; j++) {
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
            for (uint64_t i = 0; i < data.size(); i++) {
                auto chri = data[i];
                data[i] = lru_cache[chri];
                lru_cache.erase(lru_cache.begin() + chri);
                lru_cache.insert(lru_cache.begin(), data[i]);
            }
        };
    };
}

#endif
