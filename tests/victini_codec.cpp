#include <gtest/gtest.h>
#include <hzip/core/compressors/victini.h>

class VictiniCodecTest: public testing::Test {

};

TEST(VictiniCodecTest, hzip_core_compressors_victini) {
    auto mgr = new hz_memmgr;
    auto victini = hzcodec::victini();
    HZ_MEM_INIT_FROM(mgr, victini);

    auto blob = new hzblob_t;
    blob->o_data = new uint8_t[10];
    blob->o_size = 10;

    for (int i = 0; i < blob->o_size; i++) {
        blob->o_data[i] = i + 1;
    }

    hz_mstate mstate;
    auto cblob = victini.compress(blob, &mstate);
    auto dblob = victini.decompress(cblob, &mstate);


    ASSERT_EQ(dblob->o_size, blob->o_size);
    for (int i = 0; i < blob->o_size; i++) {
        ASSERT_EQ(dblob->o_data[i], blob->o_data[i]);
    }
}