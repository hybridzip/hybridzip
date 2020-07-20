#include <gtest/gtest.h>
#include <hzip/core/preprocessor/mtf_transformer.h>

class MoveToFrontTransformTest: public testing::Test {};

TEST(MoveToFrontTransformTest, hzip_core_preprocessor_mtf) {
    auto data = new int16_t[1048576];

    for (int i = 0; i < 1048576; i++) {
        data[i] = i % 256;
    }

    auto mtf = hztrans::mtf_transformer(data, 0x100, 1048576);

    mtf.transform();
    mtf.invert();

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(data[i], i % 256);
    }
}

TEST(MoveToFrontTransformTest, hzip_core_preprocessor_mtf_const) {
    auto data = new int16_t[1048576];

    for (int i = 0; i < 1048576; i++) {
        data[i] = 255;
    }

    auto mtf = hztrans::mtf_transformer(data, 0x100, 1048576);

    mtf.transform();
    mtf.invert();

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(data[i], 255);
    }
}