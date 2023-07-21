#include <gtest/gtest.h>
#include <filesystem>
#include <cmath>
#include <hzip_core/runtime/runtime.h>
#include <hzip_core/preprocessor/png_bundle.h>
#include <hzip_codec/sharingan/state_transition.h>
#include <hzip_codec/compressors.h>

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

TEST(SharinganCodecTest, sharingan_codec_test) {
    hzruntime::Config::configure();
    hzruntime::CacheProvider::init_cache("test", 1, 4194304);

#ifdef HZIP_ENABLE_OPENCL
    SharinganStateTransition::register_opencl_program();
#endif

    const char *filename = "/home/supercmmetry/Projects/hzip-research/datasets/png/7.png";

    FILE *fp = std::fopen(filename, "rb");
    auto size = std::filesystem::file_size(filename);

    auto data = rainman::ptr<uint8_t>(size);
    std::fread(data.pointer(), 1, size, fp);
    std::fclose(fp);

    auto blob = rainman::ptr<HZ_Blob>();
    blob->data = data;
    auto sharingan = hzcodec::Sharingan();

    auto start = std::chrono::high_resolution_clock::now();
    auto cblob = sharingan.compress(blob);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Actual file size: " << cblob->o_size << " bytes" << std::endl;
    std::cout << "Compressed file size: " << cblob->size << " bytes" << std::endl;
    std::cout << "Compression ratio: " << double(cblob->o_size) / double(cblob->size) << std::endl;
    std::cout << "Time taken: " << double((end - start).count()) / 1000000.0 << " ms" << std::endl;

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

