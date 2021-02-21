#include <gtest/gtest.h>
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/config.h>


class ColorTransformTest : public testing::Test {
};

#ifdef HZIP_ENABLE_OPENCL

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16) {
    Config::configure();

    uint64_t width = 1024;
    uint64_t height = 1024;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.rgb_to_ycocg(data);
    auto data_d = transformer.ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}
#endif

TEST(ColorTransformTest, hzip_color_cpu_rgb_to_ycocg16) {
    Config::configure();

    uint64_t width = 128;
    uint64_t height = 128;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);

    auto output = transformer.rgb_to_ycocg(data);
    auto data_d = transformer.ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}