#ifndef HYBRIDZIP_TRANSFORMS_H
#define HYBRIDZIP_TRANSFORMS_H

#include <vector>
#include <cstdint>

namespace hz_trans{
    class bw_transformer {

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

    class delta_transformer {
    private:
        int *data;
        uint64_t length;
    public:
        delta_transformer(int *data, uint64_t length);

        void transform();

        void invert();
    };

    class mtf_transformer {
    private:
        std::vector<uint64_t> lru_cache;
        int *data;
        uint64_t length;
        int alphabet_size;
    public:
        mtf_transformer(int *data, int alphabet_size, uint64_t length);

        void transform();

        void invert();
    };
}


#endif
