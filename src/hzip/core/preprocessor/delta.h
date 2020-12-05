#ifndef HYBRIDZIP_DELTA_H
#define HYBRIDZIP_DELTA_H

#include <vector>
#include <cstdint>

namespace hztrans {
    class DeltaTransformer {
    private:
        int *data;
        uint64_t length;
    public:
        DeltaTransformer(int *data, uint64_t length);

        void transform();

        void invert();
    };
}

#endif
