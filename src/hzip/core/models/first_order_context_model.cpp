#include "first_order_context_model.h"


hzmodels::first_order_context_model::~first_order_context_model() {
    for (int i = 0; i < alphabet_size; i++) {
        rfree(context_map[i]);
    }
    rfree(context_map);
}

void hzmodels::first_order_context_model::update(uint64_t symbol, uint64_t delta) {
    if (has_started) {
        context_map[prev_symbol][symbol] += delta;
    }
    prev_symbol = symbol;
    has_started = true;
}

uint64_t *hzmodels::first_order_context_model::get_dist(uint64_t symbol) {
    return context_map[symbol];
}

void hzmodels::first_order_context_model::set_alphabet_size(int alphabet_size) {
    context_map = rmalloc(uint64_t*, alphabet_size);
    for (int i = 0; i < alphabet_size; i++) {

        context_map[i] = rmalloc(uint64_t, alphabet_size);
        for (int j = 0; j < alphabet_size; j++) {
            context_map[i][j] = 1;
        }
    }
    prev_symbol = 0;
    this->alphabet_size = alphabet_size;
}

void hzmodels::first_order_context_model::write_to_mstate(hz_mstate *mstate) {

}


