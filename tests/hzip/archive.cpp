#include <gtest/gtest.h>
#include <hzip/archive/archive.h>
#include <hzip/utils/fsutils.h>
#include <hzip/core/compressors/victini.h>

class Archive : public testing::Test {
};

TEST(Archive, hzip_archive_init_test) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(Archive, hzip_archive_rw_file) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        auto mgr = new hz_memmgr;
        auto victini = hzcodec::victini();
        HZ_MEM_INIT_FROM(mgr, victini);
        HZ_MEM_INIT_FROM_PTR(mgr, archive);

        auto blob = new hzblob_t;
        blob->o_data = new uint8_t[20];
        blob->o_size = 20;

        for (int i = 0; i < blob->o_size; i++) {
            blob->o_data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::hz_abstract_codec *codec = &victini;

        hz_mstate mstate;
        blob->mstate = &mstate;

        auto cblob = codec->compress(blob);

        // Inject mstate into blob and add mstate to the archive.
        archive->inject_mstate(cblob->mstate, cblob);
        archive->create_file("/data.txt", cblob, 1);


        auto ccblob = archive->read_file("/data.txt");

        // compare cblob and ccblob

        ASSERT_EQ(cblob->header.length, ccblob->header.length);
        for (uint64_t i = 0; i < cblob->header.length; i++) {
            ASSERT_EQ(cblob->header.raw[i], ccblob->header.raw[i]);
        }

        ASSERT_EQ(cblob->mstate->length, ccblob->mstate->length);
        ASSERT_EQ(cblob->mstate->alg, ccblob->mstate->alg);
        for (uint64_t i = 0; i < cblob->mstate->length; i++) {
            ASSERT_EQ(cblob->mstate->data[i], ccblob->mstate->data[i]);
        }

        ASSERT_EQ(cblob->o_size, ccblob->o_size);
        ASSERT_EQ(cblob->size, ccblob->size);
        for (uint64_t i = 0; i < cblob->size; i++) {
            ASSERT_EQ(cblob->data[i], ccblob->data[i]);
        }


        auto dblob = codec->decompress(ccblob);

        ASSERT_EQ(dblob->o_size, blob->o_size);
        for (int i = 0; i < 20; i++) {
            ASSERT_EQ(dblob->o_data[i], blob->o_data[i]);
        }

        cblob->destroy();
        dblob->destroy();

        archive->close();



    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}
