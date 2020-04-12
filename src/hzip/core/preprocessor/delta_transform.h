
#ifndef HYBRIDZIP_DELTA_TRANSFORM_H
#define HYBRIDZIP_DELTA_TRANSFORM_H

#include <cstdint>

class DeltaTransformer {
private:
    int *data;
    uint64_t length;
public:
    DeltaTransformer(int *data, uint64_t length) {
        this->data = data;
        this->length = length;
    }

    void transform() {
        for (uint64_t i = length - 1; i > 0; i--) {
            data[i] -= data[i-1];
        }
    }

    void invert() {
        for (uint64_t i = 1; i < length; i++) {
            data[i] += data[i-1];
        }
    }
};

#endif
