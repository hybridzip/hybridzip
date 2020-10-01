#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip/archive/archive.h>
#include <hzip/utils/fsutils.h>
#include <hzip/core/compressors/victini.h>
#include <hzip/errors/archive.h>

class ArchiveTest : public testing::Test {
};

TEST(ArchiveTest, hzip_archive_init_test) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_rw_file) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        auto mgr = new rainman::memmgr;
        auto victini = hzcodec::victini();
        rinitfrom(mgr, victini);
        rinitptrfrom(mgr, archive);

        archive->load();

        auto blob = new hzblob_t;
        blob->o_data = new uint8_t[20];
        blob->o_size = 20;

        for (int i = 0; i < blob->o_size; i++) {
            blob->o_data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::abstract_codec *codec = &victini;

        hz_mstate mstate;
        blob->mstate = &mstate;

        auto cblob = codec->compress(blob);

        // Inject mstate into blob and add mstate to the archive.
        archive->inject_mstate(cblob->mstate, cblob);
        archive->create_file("/data.txt", cblob, 1);


        auto ccblob = &archive->read_file("/data.txt").blobs[0];

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

TEST(ArchiveTest, hzip_archive_rm_fragment_file) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        auto mgr = new rainman::memmgr;
        auto victini = hzcodec::victini();
        rinitfrom(mgr, victini);
        rinitptrfrom(mgr, archive);

        archive->load();

        auto blob = new hzblob_t;
        blob->o_data = new uint8_t[20];
        blob->o_size = 20;

        for (int i = 0; i < blob->o_size; i++) {
            blob->o_data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::abstract_codec *codec = &victini;

        hz_mstate mstate;
        blob->mstate = &mstate;

        auto cblob = codec->compress(blob);

        // Inject mstate into blob and add mstate to the archive.
        archive->install_mstate("mstate.victini.dickens", cblob->mstate);

        archive->inject_mstate("mstate.victini.dickens", cblob);

        archive->create_file("/data.txt", cblob, 1);

        archive->remove_file("/data.txt");

        archive->close();
        delete archive;
        archive = new hz_archive("test.hz");
        rinitptrfrom(mgr, archive);

        archive->load();

        EXPECT_THROW(archive->read_file("/data.txt"), ArchiveErrors::FileNotFoundException);

        archive->create_file("/data.txt", cblob, 1);

        EXPECT_THROW(archive->uninstall_mstate("mstate.victini.dickens"),
                     ArchiveErrors::InvalidOperationException);

        EXPECT_THROW(archive->uninstall_mstate(cblob->mstate_id),
                     ArchiveErrors::InvalidOperationException);


        auto ccblob = &archive->read_file("/data.txt").blobs[0];

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

        archive->remove_file("/data.txt");
        archive->uninstall_mstate("mstate.victini.dickens");

        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_rw_file_multiblob) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new hz_archive("test.hz");
        auto mgr = new rainman::memmgr;
        auto victini = hzcodec::victini();
        rinitfrom(mgr, victini);
        rinitptrfrom(mgr, archive);

        archive->load();

        auto blob = new hzblob_t;
        blob->o_data = new uint8_t[20];
        blob->o_size = 20;

        for (int i = 0; i < blob->o_size; i++) {
            blob->o_data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::abstract_codec *codec = &victini;

        hz_mstate mstate;
        blob->mstate = &mstate;

        auto cblob = codec->compress(blob);

        // Inject mstate into blob and add mstate to the archive.
        archive->install_mstate("mstate.victini.dickens", cblob->mstate);

        archive->inject_mstate("mstate.victini.dickens", cblob);

        auto blobs = new hzblob_t[2];

        blobs[0] = *cblob;
        blobs[1] = *cblob;
        archive->create_file("/data.txt", blobs, 2);

        archive->close();
        delete archive;
        archive = new hz_archive("test.hz");
        rinitptrfrom(mgr, archive);

        archive->load();

        auto ccblob = &archive->read_file("/data.txt").blobs[0];

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

        ccblob = &archive->read_file("/data.txt").blobs[1];

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

        archive->remove_file("/data.txt");
        archive->uninstall_mstate("mstate.victini.dickens");

        cblob->destroy();
        dblob->destroy();

        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}
