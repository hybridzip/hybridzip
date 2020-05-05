#include "transforms.h"

hz_trans::delta_transformer::delta_transformer(int *data, uint64_t length) {
    this->data = data;
    this->length = length;
}

void hz_trans::delta_transformer::transform() {
    for (uint64_t i = length - 1; i > 0; i--) {
        data[i] -= data[i-1];
    }
}

void hz_trans::delta_transformer::invert() {
    for (uint64_t i = 1; i < length; i++) {
        data[i] += data[i-1];
    }
}