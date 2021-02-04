#ifndef HYBRIDZIP_PAETH_H
#define HYBRIDZIP_PAETH_H

#include <cstdint>

namespace hzmodels {
    template<typename T>
    class PaethModel {
    private:
        T **arr;
        uint32_t width;
        uint32_t height;

        inline T absdiff(T x, T y) {
            return x > y ? x - y : y - x;
        }

        inline T min3(T x, T y, T z) {
            T tmp = x < y ? x : y;
            return tmp < z ? tmp : z;
        }

    public:
        /*
         * The standard Paeth Filter used in PNG compression.
         * This should only be used in conjunction with a zigzag transform.
         */
        PaethModel(T **arr, uint32_t width, uint32_t height) {
            this->arr = arr;
            this->width = width;
            this->height = height;
        }

        T predict(uint32_t x, uint32_t y) {
            if (x == 0 && y == 0) {
                return 0;
            }

            if (x == 0) {
                return arr[y - 1][x];
            }

            if (y == 0) {
                return arr[y][x - 1];
            }

            T a = arr[y - 1][x];
            T b = arr[y][x - 1];
            T c = arr[y - 1][x - 1];

            T d = a + b - c;
            T da = absdiff(a, d);
            T db = absdiff(b, d);
            T dc = absdiff(c, d);
            T dd = min3(da, db, dc);

            if (dd == da) {
                return a;
            } else if (dd == db) {
                return b;
            } else {
                return c;
            }
        }
    };
}


#endif
