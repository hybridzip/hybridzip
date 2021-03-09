#include <gtest/gtest.h>
#include <hzip_codec/sharingan/sharingan.h>
#include <hzip_codec/sharingan/state_transition.h>
#include <filesystem>
#include <hzip_core/config.h>
#include <hzip_core/runtime.h>
#include <cmath>

class SharinganCodecTest : public testing::Test {
};

TEST(SharinganCodecTest, sharingan_state_transition_cpu_precode) {
    Config::configure();
    Runtime::init_cache("test", 1, 4194304);

    const char *filename = "/home/supercmmetry/Projects/hzip-research/datasets/png/1.png";

    FILE *fp = std::fopen(filename, "rb");
    auto size = std::filesystem::file_size(filename);

    auto data = rainman::ptr<uint8_t>(size);
    std::fread(data.pointer(), 1, size, fp);
    std::fclose(fp);

    auto bundle_builder = PNGBundleBuilder(data);
    auto bundle = bundle_builder.read_bundle();

    auto sst = SharinganStateTransition(bundle);
    auto[v, mutex] = sst.cpu_dynamic_precode();

    double pbits = 0.0;
    for (uint64_t i = 0; i < v.size(); i++) {
        if (i < 10) std::cout << v[i].ls << std::endl;

        if (v[i].ls == 0) {
            std::cout << "Bad ls value at i=" << i << std::endl;
        }
        pbits += -log2(double(v[i].ls) / 16777216.0);
    }

    std::cout << "Actual size: " << size << " bytes" << std::endl;
    std::cout << "Compressed size: " << pbits / 8 << " bytes" << std::endl;
    std::cout << "Compression ratio: " << double(size * 8) / pbits << std::endl;
}
