#include "color.h"

rainman::ptr <uint8_t> hztrans::LinearU8ColorTransformer::rgb_to_ycocg(const rainman::ptr <uint8_t> &buffer) {
    if (_executor == OPENCL) {
        return opencl_rgb_to_ycocg(buffer);
    } else {
        return cpu_rgb_to_ycocg(buffer);
    }
}

rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::cpu_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) const {
    auto output = rainman::ptr<uint8_t>(buffer.size());

    auto transformer = RGBColorTransformer<uint8_t>(HZ_COLOR_TRANSFORM::RGB_TO_YCOCG);
    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto r = buffer[i];
        auto g = buffer[i + lz];
        auto b = buffer[i + lz2];

        auto pixel = ColorTransformPixel<uint8_t>{r, g, b};
        pixel = transformer.transform(pixel);

        output[i] = pixel.x;
        output[i + lz] = pixel.y;
        output[i + lz2] = pixel.z;
    }

    return output;
}

rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::opencl_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) {
    return rainman::ptr<uint8_t>();
}
