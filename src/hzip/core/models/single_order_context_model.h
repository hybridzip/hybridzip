#ifndef HYBRIDZIP_SINGLE_ORDER_CONTEXT_MODEL_H
#define HYBRIDZIP_SINGLE_ORDER_CONTEXT_MODEL_H

#include <cstdint>

class SingleOrderContextModel {
private:
    uint64_t **context_map;
    uint64_t prev_symbol;
    int alphabet_size;
    bool has_started = false;
public:
    SingleOrderContextModel(int alphabet_size) {
        context_map = new uint64_t*[alphabet_size];
        for (int i = 0; i < alphabet_size; i++) {
            context_map[i] = new uint64_t[alphabet_size];
            for (int j = 0; j < alphabet_size; j++) {
                context_map[i][j] = 0;
            }
        }
        prev_symbol = 0;
        this->alphabet_size = alphabet_size;
    }

    uint64_t* predict(uint64_t symbol) {
        return context_map[symbol];
    }

    void update(uint64_t symbol, uint64_t delta=1) {
        if (has_started) {
            context_map[prev_symbol][symbol] += delta;
        }
        prev_symbol = symbol;
        has_started = true;
    }

    ~SingleOrderContextModel() {
        for (int i = 0; i < alphabet_size; i++) {
            free(context_map[i]);
        }
        free(context_map);
    }
};

#endif
