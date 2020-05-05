#include "pivoted_gaussian_model.h"

void hzmodels_pivoted_gaussian_model::pivoted_gaussian_model::update(uint8_t byte) {
    sigma = abs(symbol - byte);
    sigma = sigma > alpha ? sigma : alpha;
    symbol = byte;
}

uint64_t hzmodels_pivoted_gaussian_model::pivoted_gaussian_model::get_cd(uint8_t byte) const {
    return gaussian_cd[sigma][byte - symbol + 128];
}

uint64_t hzmodels_pivoted_gaussian_model::pivoted_gaussian_model::get_pd(uint8_t byte) const {
    return gaussian_pd[sigma][byte - symbol + 128];
}

hzmodels_pivoted_gaussian_model::pivoted_gaussian_model::pivoted_gaussian_model(uint8_t alpha) {
    sigma = 0xff;
    symbol = 0;
    this->alpha = alpha;
}