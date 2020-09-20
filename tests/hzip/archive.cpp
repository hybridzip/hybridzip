#include <gtest/gtest.h>
#include <hzip/archive/archive.h>
#include <hzip/utils/fsutils.h>

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
