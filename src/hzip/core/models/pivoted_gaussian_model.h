#ifndef HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H
#define HYBRIDZIP_PIVOTED_GAUSSIAN_MODEL_H

#include <cstdint>
#include <cmath>
#include <hzip/utils/distribution.h>


namespace hzmodels_pivoted_gaussian_model {
    class pivoted_gaussian_model {
    private:
        int sigma, symbol;
        uint8_t alpha;
    public:
        pivoted_gaussian_model(uint8_t alpha = 0);

        uint64_t get_pd(uint8_t byte) const;

        uint64_t get_cd(uint8_t byte) const;

        void update(uint8_t byte);
    };
}

#endif
