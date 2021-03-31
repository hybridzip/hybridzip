#include <gtest/gtest.h>
#include <filesystem>
#include <cmath>
#include <hzip_core/runtime/runtime.h>
#include <hzip_core/preprocessor/png_bundle.h>
#include <hzip_codec/sharingan/state_transition.h>

class SharinganCodecTest : public testing::Test {
};

TEST(SharinganCodecTest, sharingan_state_transition_cpu_precode) {
    hzruntime::Config::configure();
    hzruntime::CacheProvider::init_cache("test", 1, 4194304);

    const char *filename = "/home/supercmmetry/Projects/hzip-research/datasets/png/7.png";

    FILE *fp = std::fopen(filename, "rb");
    auto size = std::filesystem::file_size(filename);

    auto data = rainman::ptr<uint8_t>(size);
    std::fread(data.pointer(), 1, size, fp);
    std::fclose(fp);

    auto builder = PNGBundleBuilder(data);
    auto bundle = builder.read_bundle();

    auto state_transition = SharinganStateTransition(bundle);
    state_transition.cpu_dynamic_precode();
}

#ifdef HZIP_ENABLE_OPENCL

TEST(SharinganCodecTest, sharingan_state_transition_opencl_precode) {
    hzruntime::Config::configure();
    hzruntime::CacheProvider::init_cache("test", 1, 4194304);

    const char *filename = "/home/supercmmetry/Projects/hzip-research/datasets/png/7.png";

    FILE *fp = std::fopen(filename, "rb");
    auto size = std::filesystem::file_size(filename);

    auto data = rainman::ptr<uint8_t>(size);
    std::fread(data.pointer(), 1, size, fp);
    std::fclose(fp);

    auto builder = PNGBundleBuilder(data);
    auto bundle = builder.read_bundle();

    auto state_transition = SharinganStateTransition(bundle);
    state_transition.opencl_dynamic_precode();
}

#endif