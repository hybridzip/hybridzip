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
        archive->create_file("/dir1/dir2/data.txt", cblob, 1);


        auto ccblob = &archive->read_file("/dir1/dir2/data.txt").blobs[0];

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
        archive->install_mstate("/victini/dickens", cblob->mstate);

        archive->inject_mstate("/victini/dickens", cblob);

        archive->create_file("/data.txt", cblob, 1);

        archive->remove_file("/data.txt");

        archive->close();
        delete archive;
        archive = new hz_archive("test.hz");
        rinitptrfrom(mgr, archive);

        archive->load();

        EXPECT_THROW(archive->read_file("/data.txt"), ArchiveErrors::FileNotFoundException);

        archive->create_file("/data.txt", cblob, 1);

        EXPECT_THROW(archive->uninstall_mstate("/victini/dickens"),
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
        archive->uninstall_mstate("/victini/dickens");

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
        archive->install_mstate("/victini/dickens", cblob->mstate);

        archive->inject_mstate("/victini/dickens", cblob);

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
        archive->uninstall_mstate("/victini/dickens");

        cblob->destroy();
        dblob->destroy();

        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_trie) {
    try {
        auto mgr = new rainman::memmgr;

        hza_trie<uint8_t> trie;
        rinitfrom(mgr, trie);

        trie.init();

        trie.set("/c/x1", 100);
        trie.set("/c/x2", 101);
        trie.set("/c/x3", 102);

        trie.erase("/c");

        EXPECT_THROW(trie.get("/c/x1"), ArchiveErrors::TargetNotFoundException);

        trie.set("/a/1", 1);
        trie.set("/a/2", 2);
        trie.set("/a/3", 3);
        EXPECT_THROW(trie.set("/a/1/1", 2), ArchiveErrors::InvalidOperationException);

        ASSERT_EQ(trie.get("/a/1"), 1);
        ASSERT_EQ(trie.get("/a/2"), 2);
        ASSERT_EQ(trie.get("/a/3"), 3);

        trie.set("/a/3", 4);
        ASSERT_EQ(trie.get("/a/3"), 4);

        trie.set("/a/4/1/2", 10);
        trie.erase("/a/4/1/2");

        EXPECT_THROW(trie.erase("/a/4"), ArchiveErrors::TargetNotFoundException);
        EXPECT_THROW(trie.get("/a/4/1/2"), ArchiveErrors::TargetNotFoundException);

        trie.set("/a/4", 12);
        ASSERT_EQ(trie.get("/a/4"), 12);

        trie.erase("/a");

        ASSERT_EQ(trie.children("/").size(), 0);

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}
