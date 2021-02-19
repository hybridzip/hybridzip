#include <gtest/gtest.h>
#include <hzip_codec/sharingan/sharingan.h>
#include <filesystem>

class SharinganCodecTest : public testing::Test {};

TEST(SharinganCodecTest, sharingan_test_1) {
    const char *filename = "/home/supercmmetry/Projects/hzip-research/datasets/png/3.png";

    FILE *fp = std::fopen(filename, "rb");
    auto size = std::filesystem::file_size(filename);

    auto data = rainman::ptr<uint8_t>(size);
    std::fread(data.pointer(), 1, size, fp);
    std::fclose(fp);

    auto codec = hzcodec::Sharingan();

    auto blob = rainman::ptr<HZ_Blob>();
    blob->o_size = data.size();
    blob->data = data;

    auto cblob = codec.compress(blob);

    std::cout << "Size reduction: " << (1.0 - ((double) cblob->size) / ((double) cblob->o_size)) * 100 << "%" << std::endl;
}