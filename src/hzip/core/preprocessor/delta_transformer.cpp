#include "delta_transformer.h"

hztrans::delta_transformer::delta_transformer(int *data, uint64_t length) {
    this->data = data;
    this->length = length;
}

void hztrans::delta_transformer::transform() {
    for (uint64_t i = length - 1; i > 0; i--) {
        data[i] -= data[i-1];
    }
}

void hztrans::delta_transformer::invert() {
    for (uint64_t i = 1; i < length; i++) {
        data[i] += data[i-1];
    }
}