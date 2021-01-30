#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip_storage/archive/archive.h>
#include <hzip_core/utils/fsutils.h>
#include <hzip_core/core/compressors/victini.h>
#include <hzip_storage/errors/archive.h>

class ArchiveTest : public testing::Test {
};

TEST(ArchiveTest, hzip_archive_init_test) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new HZ_Archive("test.hz");
        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_rw_file) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto *archive = new HZ_Archive("test.hz");
        auto mgr = new rainman::memmgr;
        auto victini = hzcodec::Victini();

        archive->load();

        auto blob = rainman::ptr<HZ_Blob>();
        blob->data = rainman::ptr<uint8_t>(2000);
        blob->o_size = 2000;

        for (int i = 0; i < blob->o_size; i++) {
            blob->data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::AbstractCodec *codec = &victini;

        auto mstate = rainman::ptr<HZ_MState>();
        blob->mstate = mstate;

        auto cblob = codec->compress(blob);
        cblob->evaluate(blob->data);

        // Inject mstate into blob and add mstate to the archive.
        archive->inject_mstate(cblob->mstate, cblob);
        archive->create_file("/dir1/dir2/data.txt", cblob, 1);


        auto ccblob = archive->read_file("/dir1/dir2/data.txt")[0];

        // compare cblob and ccblob

        ASSERT_EQ(cblob->header.size(), ccblob.header.size());
        for (uint64_t i = 0; i < cblob->header.size(); i++) {
            ASSERT_EQ(cblob->header.raw[i], ccblob.header.raw[i]);
        }

        if (cblob->status) {
            ASSERT_EQ(cblob->mstate->size(), ccblob.mstate->size());
            ASSERT_EQ(cblob->mstate->alg, ccblob.mstate->alg);
            for (uint64_t i = 0; i < cblob->mstate->size(); i++) {
                ASSERT_EQ(cblob->mstate->data[i], ccblob.mstate->data[i]);
            }
        }


        ASSERT_EQ(cblob->o_size, ccblob.o_size);
        ASSERT_EQ(cblob->size, ccblob.size);
        for (uint64_t i = 0; i < cblob->size; i++) {
            ASSERT_EQ(cblob->data[i], ccblob.data[i]);
        }

        auto ccblobptr = rainman::ptr<HZ_Blob>();
        *ccblobptr = ccblob;

        auto dblob = codec->decompress(ccblobptr);

        ASSERT_EQ(dblob->o_size, blob->o_size);
        for (int i = 0; i < 20; i++) {
            ASSERT_EQ(dblob->data[i], blob->data[i]);
        }

        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_rm_fragment_file) {
    try {
        fsutils::delete_file_if_exists("test.hz");

        auto archive = new HZ_Archive("test.hz");
        auto victini = hzcodec::Victini();
        archive->load();

        auto blob = rainman::ptr<HZ_Blob>();
        blob->data = rainman::ptr<uint8_t>(2000);
        blob->o_size = 2000;

        for (int i = 0; i < blob->o_size; i++) {
            blob->data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::AbstractCodec *codec = &victini;

        auto mstate = rainman::ptr<HZ_MState>();
        blob->mstate = mstate;

        auto cblob = codec->compress(blob);
        cblob->evaluate(blob->data);

        // Inject mstate into blob and add mstate to the archive.
        archive->install_mstate("/victini/dickens", cblob->mstate);

        archive->inject_mstate("/victini/dickens", cblob);

        archive->create_file("/data.txt", cblob, 1);

        archive->remove_file("/data.txt");

        archive->close();
        delete archive;
        archive = new HZ_Archive("test.hz");

        archive->load();

        EXPECT_THROW(archive->read_file("/data.txt"), ArchiveErrors::FileNotFoundException);

        archive->create_file("/data.txt", cblob, 1);

        EXPECT_THROW(archive->uninstall_mstate("/victini/dickens"),
                     ArchiveErrors::InvalidOperationException);

        EXPECT_THROW(archive->uninstall_mstate(cblob->mstate_id),
                     ArchiveErrors::InvalidOperationException);


        auto blobs = archive->read_file("/data.txt");
        auto ccblob = &blobs[0];

        // compare cblob and ccblob

        ASSERT_EQ(cblob->header.size(), ccblob->header.size());
        for (uint64_t i = 0; i < cblob->header.size(); i++) {
            ASSERT_EQ(cblob->header.raw[i], ccblob->header.raw[i]);
        }

        ASSERT_EQ(cblob->mstate->size(), ccblob->mstate->size());
        ASSERT_EQ(cblob->mstate->alg, ccblob->mstate->alg);
        for (uint64_t i = 0; i < cblob->mstate->size(); i++) {
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

        auto *archive = new HZ_Archive("test.hz");
        auto mgr = new rainman::memmgr;
        auto victini = hzcodec::Victini();

        archive->load();

        auto blob = rainman::ptr<HZ_Blob>();
        blob->data = rainman::ptr<uint8_t>(2000);
        blob->o_size = 2000;

        for (int i = 0; i < blob->o_size; i++) {
            blob->data[i] = 255;
        }

        // Upcast victini codec.
        hzcodec::AbstractCodec *codec = &victini;

        auto mstate = rainman::ptr<HZ_MState>();
        blob->mstate = mstate;

        auto cblob = codec->compress(blob);
        cblob->evaluate(blob->data);

        // Inject mstate into blob and add mstate to the archive.
        archive->install_mstate("/victini/dickens", cblob->mstate);

        archive->inject_mstate("/victini/dickens", cblob);

        auto blobs = rainman::ptr<HZ_Blob>(2);

        blobs[0] = *cblob;
        blobs[1] = *cblob;
        archive->create_file("/data.txt", blobs, 2);

        archive->close();
        delete archive;
        archive = new HZ_Archive("test.hz");

        archive->load();

        auto cblobs = archive->read_file("/data.txt");
        auto ccblob = &cblobs[0];

        // compare cblob and ccblob

        ASSERT_EQ(cblob->header.size(), ccblob->header.size());
        for (uint64_t i = 0; i < cblob->header.size(); i++) {
            ASSERT_EQ(cblob->header.raw[i], ccblob->header.raw[i]);
        }

        ASSERT_EQ(cblob->mstate->size(), ccblob->mstate->size());
        ASSERT_EQ(cblob->mstate->alg, ccblob->mstate->alg);
        for (uint64_t i = 0; i < cblob->mstate->size(); i++) {
            ASSERT_EQ(cblob->mstate->data[i], ccblob->mstate->data[i]);
        }

        ASSERT_EQ(cblob->o_size, ccblob->o_size);
        ASSERT_EQ(cblob->size, ccblob->size);
        for (uint64_t i = 0; i < cblob->size; i++) {
            ASSERT_EQ(cblob->data[i], ccblob->data[i]);
        }

        ccblob = &cblobs[1];

        // compare cblob and ccblob

        ASSERT_EQ(cblob->header.size(), ccblob->header.size());
        for (uint64_t i = 0; i < cblob->header.size(); i++) {
            ASSERT_EQ(cblob->header.raw[i], ccblob->header.raw[i]);
        }

        ASSERT_EQ(cblob->mstate->size(), ccblob->mstate->size());
        ASSERT_EQ(cblob->mstate->alg, ccblob->mstate->alg);
        for (uint64_t i = 0; i < cblob->mstate->size(); i++) {
            ASSERT_EQ(cblob->mstate->data[i], ccblob->mstate->data[i]);
        }

        ASSERT_EQ(cblob->o_size, ccblob->o_size);
        ASSERT_EQ(cblob->size, ccblob->size);
        for (uint64_t i = 0; i < cblob->size; i++) {
            ASSERT_EQ(cblob->data[i], ccblob->data[i]);
        }

        auto ccblobptr = rainman::ptr<HZ_Blob>();
        *ccblobptr = *ccblob;
        auto dblob = codec->decompress(ccblobptr);

        ASSERT_EQ(dblob->o_size, blob->o_size);
        for (int i = 0; i < 20; i++) {
            ASSERT_EQ(dblob->data[i], blob->data[i]);
        }

        archive->remove_file("/data.txt");
        archive->uninstall_mstate("/victini/dickens");

        archive->close();

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}

TEST(ArchiveTest, hzip_archive_trie) {
    try {
        HZ_ArchiveTrie<uint8_t> trie;
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

        trie.set("/rtytrydfgdfgdfgrtertfdgdfg", 1);
        trie.erase("/rtytrydfgdfgdfgrtertfdgdfg");

        ASSERT_EQ(trie.contains("/rtytrydfgdfgdfgrtertfdgdfg"), false);

    } catch (std::exception &e) {
        EXPECT_TRUE(false) << e.what();
    }
}
