#include "delta.h"

hztrans::DeltaTransformer::DeltaTransformer(int *data, uint64_t length) {
    this->data = data;
    this->length = length;
}

void hztrans::DeltaTransformer::transform() {
    for (uint64_t i = length - 1; i > 0; i--) {
        data[i] -= data[i - 1];
    }
}

void hztrans::DeltaTransformer::invert() {
    for (uint64_t i = 1; i < length; i++) {
        data[i] += data[i - 1];
    }
}