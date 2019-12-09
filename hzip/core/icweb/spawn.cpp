#include "icweb.h"

using namespace icweb;

spawn::spawn(HZIP_UINT size, float* noise_buf) {
    alphabet_size = size;
    lru_index = 1;
    this->noise_buf = noise_buf;
    freq = new HZIP_SIZE_T[size];
    total_freq = 0;
    ref_freq = 0;
    tree = hfc::fgk_tree(size);
}

spawn::~spawn() {
    free(this->freq);
    this->tree.~fgk_tree();
    free(this);
}

void spawn::update(HZIP_UINT symbol) {
    freq[symbol]++;
    total_freq++;
    ref_freq++;
    tree.update(symbol);
}

bin_t spawn::encode(HZIP_UINT symbol) {
    return tree.encode(symbol);
}

void spawn::merge(spawn* dual) {
    total_freq += dual->total_freq;
    ref_freq += dual->ref_freq;
    for(HZIP_UINT i = 0; i < alphabet_size; i++) {
        freq[i] += dual->freq[i];
    }
    tree.bulk_override(freq);
}



