#include <gtest/gtest.h>
#include <hzip/core/preprocessor/zigzag.h>

class ZigZagTransformTest: public testing::Test {};

TEST(ZigZagTransformTest, hzip_core_preprocessor_zigzag_1) {
    auto transform = hztrans::ZigZagTransformer(32, 32);

    int mat[32][32] = {0};

    for (auto iter : transform) {
        mat[iter.x][iter.y] = 1;
    }

    for (auto & i : mat) {
        for (int & j : i) {
            ASSERT_EQ(j, 1);
        }
    }
}

TEST(ZigZagTransformTest, hzip_core_preprocessor_zigzag_2) {
    auto transform = hztrans::ZigZagTransformer(32, 17);

    int mat[32][17] = {0};

    for (auto iter : transform) {
        mat[iter.x][iter.y] = 1;
    }

    for (auto & i : mat) {
        for (int & j : i) {
            ASSERT_EQ(j, 1);
        }
    }
}

TEST(ZigZagTransformTest, hzip_core_preprocessor_zigzag_3) {
    auto transform = hztrans::ZigZagTransformer(17, 32);

    int mat[17][32] = {0};

    for (auto iter : transform) {
        mat[iter.x][iter.y] = 1;
    }

    for (auto & i : mat) {
        for (int & j : i) {
            ASSERT_EQ(j, 1);
        }
    }
}

TEST(ZigZagTransformTest, hzip_core_preprocessor_zigzag_4) {
    auto transform = hztrans::ZigZagTransformer(17, 17);

    int mat[17][17] = {0};

    for (auto iter : transform) {
        mat[iter.x][iter.y] = 1;
    }

    for (auto & i : mat) {
        for (int & j : i) {
            ASSERT_EQ(j, 1);
        }
    }
}
