#ifndef HYBRIDZIP_COLOR_TRANSFORM_H
#define HYBRIDZIP_COLOR_TRANSFORM_H

#include <cstdint>
#include <hzip_core/errors/transform.h>
#include <hzip_core/executor.h>
#include <rainman/rainman.h>

namespace hztrans {
    enum HZ_COLOR_SPACE {
        YCOCG = 0x0,
        YCBCR = 0x1,
    };

    enum HZ_COLOR_TRANSFORM {
        RGB_TO_YCOCG = 0x0,
        YCOCG_TO_RGB = 0x1,
        RGB_TO_YCBCR = 0x2,
        YCBCR_TO_RGB = 0x3
    };

    template<typename T>
    struct ColorTransformPixel {
        T x;
        T y;
        T z;
    };

    template<typename T>
    class RGBColorTransformer {
    private:
        HZ_COLOR_TRANSFORM transform_type;

        template<typename PairType>
        struct pair {
            PairType x;
            PairType y;
        };

        inline pair<T> forward_lift(pair<T> p) {
            T diff = (p.y - p.x);
            T average = (p.x + (diff >> 1));
            return pair<T>{.x=average, .y=diff};
        }

        inline pair<T> reverse_lift(pair<T> p) {
            T x = (p.x - (p.y >> 1));
            T y = (x + p.y);
            return pair<T>{.x=x, .y=y};
        }

    public:
        RGBColorTransformer(HZ_COLOR_TRANSFORM transform_type) {
            this->transform_type = transform_type;
        }

        RGBColorTransformer(HZ_COLOR_SPACE color_space, bool reverse = false) {
            this->transform_type = static_cast<HZ_COLOR_TRANSFORM>((color_space << 1) + reverse);
        }

        ColorTransformPixel<T> transform(ColorTransformPixel<T> pixel) {
            switch (transform_type) {
                case RGB_TO_YCOCG: {
                    auto red = pixel.x;
                    auto green = pixel.y;
                    auto blue = pixel.z;

                    pair temp_co = forward_lift(pair<T>{.x=red, .y=blue});
                    pair y_cg = forward_lift(pair<T>{.x=green, .y=temp_co.x});
                    return ColorTransformPixel<T>{.x=y_cg.x, .y=y_cg.y, .z=temp_co.y};
                }
                case YCOCG_TO_RGB: {
                    auto y = pixel.x;
                    auto co = pixel.y;
                    auto cg = pixel.z;

                    pair green_temp = reverse_lift(pair<T>{.x=y, .y=cg});
                    pair red_blue = reverse_lift(pair<T>{.x=green_temp.y, .y=co});
                    return ColorTransformPixel<T>{.x=red_blue.x, .y=green_temp.x, .z=red_blue.y};

                }
                default:
                    throw TransformErrors::InvalidInputError("Undefined transform");
            }
        }
    };

    class LinearU8ColorTransformer {
    private:
        Executor _executor;
        uint64_t _width;
        uint64_t _height;

#ifdef HZIP_ENABLE_OPENCL
        static void register_kernel();

        [[nodiscard]] rainman::ptr<uint8_t> opencl_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) const;
#endif

        [[nodiscard]] rainman::ptr<uint8_t> cpu_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) const;
    public:
        LinearU8ColorTransformer(
                uint64_t width,
                uint64_t height,
                Executor executor = get_best_executor()
        ) : _width(width), _height(height), _executor(executor) {}

        rainman::ptr<uint8_t> rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer);

        rainman::ptr<uint8_t> ycocg_to_rgb(const rainman::ptr<uint8_t> &buffer);
    };

}

#endif
