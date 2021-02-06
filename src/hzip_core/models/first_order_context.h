#ifndef HYBRIDZIP_FIRST_ORDER_CONTEXT_H
#define HYBRIDZIP_FIRST_ORDER_CONTEXT_H

#include <cstdint>
#include <cmath>
#include <rainman/rainman.h>

namespace hzmodels {
    class FirstOrderContextModel {
    private:
        rainman::ptr2d<uint64_t> _context_map;
        uint64_t _prev_symbol{};
        bool _has_started = false;
    public:

        FirstOrderContextModel() = default;

        void set_alphabet_size(uint64_t alphabet_size);

        rainman::ptr<uint64_t> get_dist(uint64_t symbol);

        void update(uint64_t symbol, uint64_t delta = 1);

        void update(uint64_t prev_symbol, uint64_t next_symbol, uint64_t delta);

        void revert(uint64_t prev_symbol, uint64_t next_symbol, uint64_t delta);
    };
}

#endif
