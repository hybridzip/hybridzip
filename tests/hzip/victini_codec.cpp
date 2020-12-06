#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip/core/compressors/victini.h>

class VictiniCodecTest: public testing::Test {};

TEST(VictiniCodecTest, hzip_core_compressors_victini_small) {
    auto mgr = new rainman::memmgr;
    auto victini = hzcodec::Victini();
    rinitfrom(mgr, victini);

    auto blob = new HZ_Blob;
    blob->data = new uint8_t[20];
    blob->o_size = 20;

    for (int i = 0; i < blob->o_size; i++) {
        blob->data[i] = 255;
    }

    // Upcast victini codec.
    hzcodec::AbstractCodec *codec = &victini;

    auto cblob = codec->compress(blob);
    auto dblob = codec->decompress(cblob);
    auto ccblob = codec->compress(dblob);
    auto ddblob = codec->decompress(ccblob);

    ASSERT_EQ(dblob->o_size, blob->o_size);
    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(dblob->data[i], blob->data[i]);
        ASSERT_EQ(ddblob->data[i], blob->data[i]);
    }

    cblob->destroy();
    ccblob->destroy();
    dblob->destroy();
    ddblob->destroy();
}

TEST(VictiniCodecTest, hzip_core_compressors_victini_large) {
    auto mgr = new rainman::memmgr;
    auto victini = hzcodec::Victini();
    rinitfrom(mgr, victini);

    auto blob = new HZ_Blob;
    blob->data = new uint8_t[1048576];
    blob->o_size = 1048576;

    for (int i = 0; i < blob->o_size; i++) {
        blob->data[i] = i % 256;
    }

    // Upcast victini codec.
    hzcodec::AbstractCodec *codec = &victini;

    auto cblob = codec->compress(blob);
    auto dblob = codec->decompress(cblob);
    auto ccblob = codec->compress(dblob);
    auto ddblob = codec->decompress(ccblob);

    ASSERT_EQ(dblob->o_size, blob->o_size);
    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(dblob->data[i], blob->data[i]);
    }

    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(ddblob->data[i], blob->data[i]);
    }

    cblob->destroy();
    ccblob->destroy();
    dblob->destroy();
    ddblob->destroy();
}