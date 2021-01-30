#include <gtest/gtest.h>
#include <hzip_core/core/preprocessor/move_to_front.h>

class MoveToFrontTransformTest: public testing::Test {};

TEST(MoveToFrontTransformTest, hzip_core_preprocessor_mtf) {
    auto data = rainman::ptr<int16_t>(1048576);

    for (int i = 0; i < 1048576; i++) {
        data[i] = i % 256;
    }

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100);

    mtf.transform();
    mtf.invert();

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(data[i], i % 256);
    }
}

TEST(MoveToFrontTransformTest, hzip_core_preprocessor_mtf_const) {
    auto data = rainman::ptr<int16_t>(1048576);

    for (int i = 0; i < 1048576; i++) {
        data[i] = 255;
    }

    auto mtf = hztrans::MoveToFrontTransformer(data, 0x100);

    mtf.transform();
    mtf.invert();

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(data[i], 255);
    }
}