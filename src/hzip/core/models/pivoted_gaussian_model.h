#include <cstdint>

#ifndef HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H
#define HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H

#include <hzip/utils/distribution.h>
#define PIVOTED_GAUSSIAN_MODEL_SCALE 24

class PivotedGaussianModel {
private:
    int sigma, symbol;
    uint8_t alpha;
public:
    PivotedGaussianModel(uint8_t alpha = 0) {
        sigma = 0xff;
        symbol = 0;
        this->alpha = alpha;
    }

    uint64_t get_pd(uint8_t byte) const {
        return gaussian_pd[sigma][byte - symbol + 128];
    }

    uint64_t get_cd(uint8_t byte) const {
        return gaussian_cd[sigma][byte - symbol + 128];
    }

    void update(uint8_t byte) {
        sigma = abs(symbol - byte);
        sigma = sigma > alpha ? sigma : alpha;
        symbol = byte;
    }
};

#endif
