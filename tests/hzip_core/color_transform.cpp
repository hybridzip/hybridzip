#include <gtest/gtest.h>
#include <hzip_core/preprocessor/transforms.h>
#include <hzip_core/runtime/runtime.h>


class ColorTransformTest : public testing::Test {
};

#ifdef HZIP_ENABLE_OPENCL

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16) {
    hzruntime::Config::configure();

    uint64_t width = 1024;
    uint64_t height = 1024;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.opencl_rgb_to_ycocg(data);
    auto data_d = transformer.opencl_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_1) {
    hzruntime::Config::configure();

    uint64_t width = 123;
    uint64_t height = 132;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.opencl_rgb_to_ycocg(data);
    auto data_d = transformer.opencl_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_2) {
    hzruntime::Config::configure();

    uint64_t width = 123;
    uint64_t height = 132;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.opencl_rgb_to_ycocg(data);
    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_3) {
    hzruntime::Config::configure();

    uint64_t width = 123;
    uint64_t height = 132;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.cpu_rgb_to_ycocg(data);
    auto data_d = transformer.opencl_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_4) {
    hzruntime::Config::configure();

    uint64_t width = 123;
    uint64_t height = 132;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.cpu_rgb_to_ycocg(data);
    auto data_d = transformer.opencl_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_5) {
    hzruntime::Config::configure();

    uint64_t width = 123;
    uint64_t height = 132;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.opencl_rgb_to_ycocg(data);
    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_opencl_rgb_to_ycocg16_6) {
    hzruntime::Config::configure();

    uint64_t width = 1024;
    uint64_t height = 1024;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16ColorTransformer(width, height);
    auto output = transformer.opencl_rgb_to_ycocg(data);
    auto data_d = transformer.opencl_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}
#endif

TEST(ColorTransformTest, hzip_color_u16x_cpu_rgb_to_ycocg_depth_16) {
    hzruntime::Config::configure();

    uint64_t width = 128;
    uint64_t height = 128;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xffff;
    }

    auto transformer = hztrans::LinearU16XColorTransformer(width, height, 16);

    auto output = transformer.cpu_rgb_to_ycocg(data);
    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_u16x_cpu_rgb_to_ycocg_depth_8) {
    hzruntime::Config::configure();

    uint64_t width = 113;
    uint64_t height = 127;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        data[i] = i & 0xff;
    }

    auto transformer = hztrans::LinearU16XColorTransformer(width, height, 8);

    auto output = transformer.cpu_rgb_to_ycocg(data);

    for (int i = 0; i < output.size(); i++) {
        ASSERT_TRUE(output[i] <= 0xff);
    }

    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_u16x_cpu_rgb_to_ycocg_depth_4) {
    hzruntime::Config::configure();

    uint64_t width = 37;
    uint64_t height = 73;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0xf;
    }

    auto transformer = hztrans::LinearU16XColorTransformer(width, height, 4);

    auto output = transformer.cpu_rgb_to_ycocg(data);

    for (int i = 0; i < output.size(); i++) {
        ASSERT_TRUE(output[i] <= 0xf);
    }

    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_u16x_cpu_rgb_to_ycocg_depth_2) {
    hzruntime::Config::configure();

    uint64_t width = 37;
    uint64_t height = 73;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0x3;
    }

    auto transformer = hztrans::LinearU16XColorTransformer(width, height, 2);

    auto output = transformer.cpu_rgb_to_ycocg(data);

    for (int i = 0; i < output.size(); i++) {
        ASSERT_TRUE(output[i] <= 0x3);
    }

    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}

TEST(ColorTransformTest, hzip_color_u16x_cpu_rgb_to_ycocg_depth_1) {
    hzruntime::Config::configure();

    uint64_t width = 37;
    uint64_t height = 73;

    auto data = rainman::ptr<uint16_t>(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        data[i] = i & 0x1;
    }

    auto transformer = hztrans::LinearU16XColorTransformer(width, height, 1);

    auto output = transformer.cpu_rgb_to_ycocg(data);

    for (int i = 0; i < output.size(); i++) {
        ASSERT_TRUE(output[i] <= 0x1);
    }

    auto data_d = transformer.cpu_ycocg_to_rgb(output);

    for (int i = 0; i < data.size(); i++) {
        ASSERT_EQ(data_d[i], data[i]);
    }
}