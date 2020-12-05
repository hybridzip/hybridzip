#ifndef HYBRIDZIP_FIRST_ORDER_CONTEXT_MODEL_H
#define HYBRIDZIP_FIRST_ORDER_CONTEXT_MODEL_H

#include <cstdint>
#include <cmath>
#include <rainman/rainman.h>

namespace hzmodels {
    class first_order_context_model : public rainman::context {
    private:
        uint64_t **context_map;
        uint64_t prev_symbol;
        int alphabet_size;
        bool has_started = false;
    public:

        first_order_context_model() {
            // empty-constructor
        }

        void set_alphabet_size(int alphabet_size);

        uint64_t *get_dist(uint64_t symbol);

        void update(uint64_t symbol, uint64_t delta = 1);

        ~first_order_context_model();
    };
}

#endif
