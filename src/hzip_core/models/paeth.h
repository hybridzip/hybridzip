#ifndef HYBRIDZIP_PAETH_H
#define HYBRIDZIP_PAETH_H

#include <cstdint>
#include <rainman/rainman.h>

namespace hzmodels {
    template<typename T>
    class PaethModel {
    private:
        rainman::ptr<T> arr;
        uint32_t width;
        uint32_t height;
        uint64_t channel_shift;

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
         */
        PaethModel(const rainman::ptr<T> &arr, uint32_t width, uint32_t height, uint64_t channel_index) {
            this->arr = arr;
            this->width = width;
            this->height = height;
            this->channel_shift = channel_index * width * height;
        }

        T predict(uint32_t x, uint32_t y) {
            if (x == 0 && y == 0) {
                return 0;
            }

            if (x == 0) {
                return arr[channel_shift + (y - 1) * width + x];
            }

            if (y == 0) {
                return arr[channel_shift + y * width + x - 1];
            }

            T a = arr[channel_shift + (y - 1) * width + x];
            T b = arr[channel_shift + y * width + x - 1];
            T c = arr[channel_shift + (y - 1) * width + x - 1];

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

    class LinearU16PaethDifferential {
    private:
        static inline uint32_t absdiff(uint32_t x, uint32_t y) {
            return x > y ? x - y : y - x;
        }

        static inline uint32_t min3(uint32_t x, uint32_t y, uint32_t z) {
            uint32_t tmp = x < y ? x : y;
            return tmp < z ? tmp : z;
        }

    public:
        LinearU16PaethDifferential() = default;

        static rainman::ptr<uint16_t> filter(
                const rainman::ptr<uint16_t> &buffer,
                uint64_t width,
                uint64_t height,
                uint64_t nchannels,
                bool inplace = false,
                uint8_t bit_depth = 16
        );

        static rainman::ptr<uint16_t> cpu_filter(
                const rainman::ptr<uint16_t> &buffer,
                uint64_t width,
                uint64_t height,
                uint64_t nchannels,
                bool inplace = false,
                uint8_t bit_depth = 16
        );

#ifdef HZIP_ENABLE_OPENCL
        static void register_opencl_program();

        static rainman::ptr<uint16_t> opencl_filter(
                const rainman::ptr<uint16_t> &buffer,
                uint64_t width,
                uint64_t height,
                uint64_t nchannels,
                bool inplace = false,
                uint8_t bit_depth = 16
        );
#endif
    };
}


#endif
