#include <gtest/gtest.h>
#include <hzip/core/compressors/victini.h>

class VictiniCodecTest: public testing::Test {

};

TEST(VictiniCodecTest, hzip_core_compressors_victini) {
    auto mgr = new hz_memmgr;
    auto victini = hzcodec::victini();
    HZ_MEM_INIT_FROM(mgr, victini);

    auto blob = new hzblob_t;
    blob->o_data = new uint8_t[100];
    blob->o_size = 100;

    for (int i = 0; i < blob->o_size; i++) {
        blob->o_data[i] = i + 1;
    }

    // Upcast victini codec.
    hzcodec::hz_abstract_codec *codec = &victini;

    hz_mstate mstate;
    blob->mstate = &mstate;

    auto cblob = codec->compress(blob);
    auto dblob = codec->decompress(cblob);
    auto ccblob = codec->compress(dblob);
    auto ddblob = codec->decompress(ccblob);

    ASSERT_EQ(dblob->o_size, blob->o_size);
    for (int i = 0; i < blob->o_size; i++) {
        ASSERT_EQ(dblob->o_data[i], blob->o_data[i]);
        ASSERT_EQ(ddblob->o_data[i], blob->o_data[i]);
    }

    cblob->destroy();
    ccblob->destroy();
    dblob->destroy();
    ddblob->destroy();
}