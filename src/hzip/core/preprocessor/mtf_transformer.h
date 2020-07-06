#ifndef HYBRIDZIP_MTF_TRANSFORMER_H
#define HYBRIDZIP_MTF_TRANSFORMER_H

#include <vector>
#include <cstdint>

namespace hztrans {
    class mtf_transformer {
    private:
        std::vector<uint64_t> lru_cache;
        int *data;
        uint64_t length;
        int alphabet_size;
    public:
        mtf_transformer(int *data, int alphabet_size, uint64_t length);

        void transform();

        void invert();
    };
}

#endif
