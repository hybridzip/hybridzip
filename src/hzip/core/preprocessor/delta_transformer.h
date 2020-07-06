#ifndef HYBRIDZIP_DELTA_TRANSFORMER_H
#define HYBRIDZIP_DELTA_TRANSFORMER_H

#include <vector>
#include <cstdint>

namespace hztrans {
    class delta_transformer {
    private:
        int *data;
        uint64_t length;
    public:
        delta_transformer(int *data, uint64_t length);

        void transform();

        void invert();
    };
}

#endif
