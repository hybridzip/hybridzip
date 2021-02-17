#include <iostream>
#include <chrono>
#include <gtest/gtest.h>
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/config.h>


class ColorTransformTest : public testing::Test {
};

#ifdef HZIP_ENABLE_OPENCL

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg) {
    Config::configure();

    uint64_t width = 1024;
    uint64_t height = 1024;

    auto data = rainman::ptr<uint8_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xff;
    }

    auto cpu_transformer = hztrans::LinearU8ColorTransformer(width, height, CPU);
    auto opencl_transformer = hztrans::LinearU8ColorTransformer(width, height, OPENCL);

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto output1 = cpu_transformer.rgb_to_ycocg(data);

    std::cout << "CPU: " << (double) (clock.now() - start).count() / 1000000000.0 << "s" << std::endl;

    start = clock.now();

    auto output2 = opencl_transformer.rgb_to_ycocg(data);

    std::cout << "OPENCL: " << (double) (clock.now() - start).count() / 1000000000.0 << "s" << std::endl;

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(output1[i], output2[i]);
    }

}

#endif