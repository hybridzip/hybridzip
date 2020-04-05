#include <cstdint>

#ifndef HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H
#define HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H

#include <hzip/utils/distribution.h>
#define PIVOTED_GAUSSIAN_MODEL_SCALE 24

class PivotedGaussianModel {
private:
    int sigma, symbol;
public:
    PivotedGaussianModel() {
        sigma = 0xff;
        symbol = 0;
    }

    uint64_t get_pd(uint8_t byte) const {
        return gaussian_pd[sigma][byte - symbol + 128];
    }

    uint64_t get_cd(uint8_t byte) const {
        return gaussian_cd[sigma][byte - symbol + 128];
    }

    void update(uint8_t byte) {
        sigma = abs(symbol - byte);
        // sigma = sigma > 32 ? sigma : 32;
        symbol = byte;
    }
};

#endif
