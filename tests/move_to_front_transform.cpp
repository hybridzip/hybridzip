#include <gtest/gtest.h>
#include <hzip/core/preprocessor/mtf_transformer.h>

class MoveToFrontTransformTest: public testing::Test {};


TEST(MoveToFrontTransformTest, hzip_core_preprocessor_mtf) {
    auto data = new int16_t[20];

    for (int i = 0; i < 20; i++) {
        data[i] = i + 1;
    }

    auto mtf = hztrans::mtf_transformer<int16_t>(data, 0x100, 20);

    mtf.transform();
    mtf.invert();

    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(data[i], i + 1);
    }
}