#include <gtest/gtest.h>
#include <hzip/core/preprocessor/transforms.h>

class BurrowsWheelerTransformTest: public testing::Test {};

TEST(BurrowsWheelerTransformTest, hzip_core_preprocessor_bwt) {
    auto data = new int16_t[20];

    for (int i = 0; i < 20; i++) {
        data[i] = i + 1;
    }

    auto bwt = hztrans::bw_transformer(data, 20, 0x100);
    bwt._hz_memmgr_attach_memmgr(new hz_memmgr);

    auto index = bwt.transform();

    bwt.invert(index);

    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(data[i], i + 1);
    }
}

