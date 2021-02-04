#include "first_order_context.h"

void hzmodels::FirstOrderContextModel::update(uint64_t symbol, uint64_t delta) {
    if (_has_started) {
        _context_map[_prev_symbol][symbol] += delta;
    }
    _prev_symbol = symbol;
    _has_started = true;
}

rainman::ptr<uint64_t> hzmodels::FirstOrderContextModel::get_dist(uint64_t symbol) {
    return _context_map[symbol];
}

void hzmodels::FirstOrderContextModel::set_alphabet_size(uint64_t alphabet_size) {
    _context_map = rainman::make_ptr2d<uint64_t>(alphabet_size, alphabet_size);
    for (auto i = 0; i < alphabet_size; i++) {
        for (auto j = 0; j < alphabet_size; j++) {
            _context_map[i][j] = 1;
        }
    }
    _prev_symbol = 0;
}


