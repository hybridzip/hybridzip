#ifndef HYBRIDZIP_FIRST_ORDER_CONTEXT_MODEL_H
#define HYBRIDZIP_FIRST_ORDER_CONTEXT_MODEL_H

#include <cstdint>
#include <cmath>

namespace hzmodels_first_order_context_model {
    class first_order_context_model {
    private:
        uint64_t **context_map;
        uint64_t prev_symbol;
        int alphabet_size;
        bool has_started = false;
    public:
        first_order_context_model(int alphabet_size);

        uint64_t *predict(uint64_t symbol);

        void update(uint64_t symbol, uint64_t delta = 1);

        ~first_order_context_model();
    };
}

#endif
