#include "first_order_context_model.h"


hzmodels_first_order_context_model::first_order_context_model::~first_order_context_model() {
    for (int i = 0; i < alphabet_size; i++) {
        free(context_map[i]);
    }
    free(context_map);
}

void hzmodels_first_order_context_model::first_order_context_model::update(uint64_t symbol, uint64_t delta) {
    if (has_started) {
        context_map[prev_symbol][symbol] += delta;
    }
    prev_symbol = symbol;
    has_started = true;
}

uint64_t *hzmodels_first_order_context_model::first_order_context_model::predict(uint64_t symbol) {
    return context_map[symbol];
}

hzmodels_first_order_context_model::first_order_context_model::first_order_context_model(int alphabet_size) {
    context_map = new uint64_t*[alphabet_size];
    for (int i = 0; i < alphabet_size; i++) {
        context_map[i] = new uint64_t[alphabet_size];
        for (int j = 0; j < alphabet_size; j++) {
            context_map[i][j] = 1;
        }
    }
    prev_symbol = 0;
    this->alphabet_size = alphabet_size;
}


