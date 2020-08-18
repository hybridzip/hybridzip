#include <gtest/gtest.h>
#include <hzip/core/preprocessor/transforms.h>

class BurrowsWheelerTransformTest: public testing::Test {};

TEST(BurrowsWheelerTransformTest, hzip_core_preprocessor_bwt_large) {
    auto N = 1048576;
    auto data = new int16_t[N];

    for (int i = 0; i < N; i++) {
        data[i] = i % 256;
    }

    auto bwt = hztrans::bw_transformer<int16_t, int32_t>(data, N, 0x100);
    bwt._hz_memmgr_attach_memmgr(new hz_memmgr);

    auto index = bwt.transform();

    for (int i = 0; i < N; i++) {
        ASSERT_FALSE(data[i] < 0 || data[i] > 255);
    }

    bwt.invert(index);

    for (int i = 0; i < N; i++) {
        ASSERT_EQ(data[i], i % 256);
    }
}

TEST(BurrowsWheelerTransformTest, hzip_core_preprocessor_bwt_small) {
    int N = 64;
    auto data = new int16_t[N];

    for (int i = 0; i < 64; i++) {
        data[i] = (255 + i) % 256;
    }


    auto bwt = hztrans::bw_transformer<int16_t, int32_t>(data, N, 0x100);
    bwt._hz_memmgr_attach_memmgr(new hz_memmgr);

    auto index = bwt.transform();

    for (int i = 0; i < N; i++) {
        ASSERT_FALSE(data[i] < 0 || data[i] > 255);
    }

    bwt.invert(index);


    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(data[i], (255 + i) % 256);
    }
}

TEST(BurrowsWheelerTransformTest, hzip_core_preprocessor_bwt_const) {
    int N = 1024;
    auto data = new int16_t[N];

    for (int i = 0; i < N; i++) {
        data[i] = 255;
    }


    auto bwt = hztrans::bw_transformer<int16_t, int32_t>(data, N, 0x100);
    bwt._hz_memmgr_attach_memmgr(new hz_memmgr);

    auto index = bwt.transform();

    for (int i = 0; i < N; i++) {
        ASSERT_FALSE(data[i] < 0 || data[i] > 255);
    }

    bwt.invert(index);


    for (int i = 0; i < N; i++) {
        ASSERT_EQ(data[i], 255);
    }
}
