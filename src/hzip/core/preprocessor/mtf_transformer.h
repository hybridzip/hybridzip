#ifndef HYBRIDZIP_MTF_TRANSFORMER_H
#define HYBRIDZIP_MTF_TRANSFORMER_H

#include <vector>
#include <cstdint>

namespace hztrans {

    template<typename itype>
    class mtf_transformer {
    private:
        std::vector<itype> lru_cache;
        itype *data;
        uint64_t length{};
        int alphabet_size{};
    public:
        mtf_transformer(itype *data, int alphabet_size, uint64_t length) {
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
                auto symbol = data[i];
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
        };
    };
}

#endif
