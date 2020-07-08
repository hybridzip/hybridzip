#ifndef HYBRIDZIP_BW_TRANSFORMER_H
#define HYBRIDZIP_BW_TRANSFORMER_H

#include <vector>
#include <cstdint>
#include <hzip/memory/mem_interface.h>

namespace hztrans {
    class bw_transformer: public hz_mem_iface {

    private:
        static inline bool leq(int a1, int a2, int b1, int b2) {
            return (a1 < b1 || a1 == b1 && a2 <= b2);
        }

        static inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
            return (a1 < b1 || a1 == b1 && leq(a2, a3, b2, b3));
        }

        void radix_pass(int *a, int *b, int *r, int n, int K);

        void suffix_array(int *T, int *SA, int n, int K);

        int *data, len, alphabet_size;
    public:
        bw_transformer(int *data, int n, int K);

        int transform();

        void invert(int bw_index);
    };
}
#endif
