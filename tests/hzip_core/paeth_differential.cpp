#include <chrono>
#include <gtest/gtest.h>
#include <hzip_core/models/paeth.h>
#include <hzip_core/runtime/runtime.h>

class PaethDifferentialTest : public testing::Test {
};

#ifdef HZIP_ENABLE_OPENCL

TEST(PaethDifferentialTest, opencl_paeth_diff_test_1) {
    hzruntime::Config::configure();

    uint64_t width = 1920;
    uint64_t height = 1080;
    uint64_t nchannels = 3;

    auto buffer = rainman::ptr<uint16_t>(width * height * nchannels);

    for (uint64_t i = 0; i < buffer.size(); i++) {
        buffer[i] = i & 0xff;
    }

    hzmodels::LinearU16PaethDifferential::register_opencl_program();

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto cpu_output = hzmodels::LinearU16PaethDifferential::cpu_filter(buffer, width, height, nchannels);

    std::cout << "[opencl_paeth_diff_test_1] CPU time: " << double((clock.now() - start).count()) / 1000000000.0 << " s"
              << std::endl;

    start = clock.now();

    auto opencl_output = hzmodels::LinearU16PaethDifferential::opencl_filter(buffer, width, height, nchannels);

    std::cout << "[opencl_paeth_diff_test_1] OPENCL time: " << double((clock.now() - start).count()) / 1000000000.0
              << " s" << std::endl;

    for (uint64_t i = 0; i < buffer.size(); i++) {
        ASSERT_EQ(cpu_output[i], opencl_output[i]);
    }
}

TEST(PaethDifferentialTest, opencl_paeth_diff_test_2) {
    hzruntime::Config::configure();

    uint64_t width = 1;
    uint64_t height = 1;
    uint64_t nchannels = 3;

    auto buffer = rainman::ptr<uint16_t>(width * height * nchannels);

    for (uint64_t i = 0; i < buffer.size(); i++) {
        buffer[i] = i & 0xff;
    }

    hzmodels::LinearU16PaethDifferential::register_opencl_program();

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto cpu_output = hzmodels::LinearU16PaethDifferential::cpu_filter(buffer, width, height, nchannels);

    std::cout << "[opencl_paeth_diff_test_2] CPU time: " << double((clock.now() - start).count()) / 1000000000.0 << " s"
              << std::endl;

    start = clock.now();

    auto opencl_output = hzmodels::LinearU16PaethDifferential::opencl_filter(buffer, width, height, nchannels);

    std::cout << "[opencl_paeth_diff_test_2] OPENCL time: " << double((clock.now() - start).count()) / 1000000000.0
              << " s" << std::endl;

    for (uint64_t i = 0; i < buffer.size(); i++) {
        ASSERT_EQ(cpu_output[i], opencl_output[i]);
    }
}

TEST(PaethDifferentialTest, opencl_paeth_diff_test_3) {
    hzruntime::Config::configure();

    uint64_t width = 1;
    uint64_t height = 16384;
    uint64_t nchannels = 3;

    auto buffer = rainman::ptr<uint16_t>(width * height * nchannels);
    auto buffer_copy = rainman::ptr<uint16_t>(width * height * nchannels);

    for (uint64_t i = 0; i < buffer.size(); i++) {
        buffer[i] = i & 0xff;
        buffer_copy[i] = i & 0xff;
    }

    hzmodels::LinearU16PaethDifferential::register_opencl_program();

    auto clock = std::chrono::high_resolution_clock();
    auto start = clock.now();

    auto cpu_output = hzmodels::LinearU16PaethDifferential::cpu_filter(buffer, width, height, nchannels, true);

    std::cout << "[opencl_paeth_diff_test_3] CPU time: " << double((clock.now() - start).count()) / 1000000000.0 << " s"
              << std::endl;

    start = clock.now();

    auto opencl_output = hzmodels::LinearU16PaethDifferential::opencl_filter(buffer_copy, width, height, nchannels);

    std::cout << "[opencl_paeth_diff_test_3] OPENCL time: " << double((clock.now() - start).count()) / 1000000000.0
              << " s" << std::endl;

    for (uint64_t i = 0; i < buffer.size(); i++) {
        ASSERT_EQ(cpu_output[i], opencl_output[i]);
    }
}

#endif