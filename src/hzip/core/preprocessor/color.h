#ifndef HYBRIDZIP_COLOR_TRANSFORM_H
#define HYBRIDZIP_COLOR_TRANSFORM_H

#include <cstdint>

namespace hztrans {
    enum HZ_COLOR_TRANSFORM {
        RGB_TO_YCOCG,
        YCOCG_TO_RGB,
    };

    template<typename T>
    struct ColorTransformPixel {
        T x;
        T y;
        T z;
    };

    template<typename T>
    class ColorTransformer {
    private:
        HZ_COLOR_TRANSFORM transform_type;

        template<typename PairType>
        struct pair {
            PairType x;
            PairType y;
        };

        pair<T> forward_lift(pair<T> p) {
            T diff = (p.y - p.x);
            T average = (p.x + (diff >> 1));
            return pair{.x=average, .y=diff};
        }

        pair<T> reverse_lift(pair<T> p) {
            T x = (p.x - (p.y >> 1));
            T y = (x + p.y);
            return pair{.x=x, .y=y};
        }

    public:
        ColorTransformer(HZ_COLOR_TRANSFORM transform_type) {
            this->transform_type = transform_type;
        }

        ColorTransformPixel<T> transform(ColorTransformPixel<T> pixel) {
            switch (transform_type) {
                case RGB_TO_YCOCG: {
                    auto red = pixel.x;
                    auto green = pixel.y;
                    auto blue = pixel.z;

                    pair temp_co = forward_lift(red, blue);
                    pair y_cg = forward_lift(green, temp_co.x);
                    return ColorTransformPixel{.x=y_cg.x, .y=y_cg.y, .z=temp_co.y};
                }
                case YCOCG_TO_RGB: {
                    auto y = pixel.x;
                    auto co = pixel.y;
                    auto cg = pixel.z;

                    pair green_temp = reverse_lift(y, cg);
                    pair red_blue = reverse_lift(green_temp.y, co);
                    return ColorTransformPixel{.x=red_blue.x, .y=green_temp.x, .z=red_blue.y};
                }
            }
        }
    };
}

#endif