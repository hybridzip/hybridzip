#include <iostream>
#include <hzip/core/compressors/white_rose.h>
#include <hzip/memory/memmgr.h>
#include <loguru/loguru.hpp>

void setup_logger(int argc, char **argv) {
#ifndef DEBUG
    loguru::g_preamble_file = false;
#endif

    loguru::init(argc, argv);
}

void set_unhandled_exception_handler() {
    std::set_terminate([]() {
        LOG_F(ERROR, "hzip: encountered unhandled exception");
        abort();
    });
}

int main(int argc, char **argv) noexcept {

    set_unhandled_exception_handler();
    setup_logger(argc, argv);

    LOG_F(INFO, "hzip: started hybridzip server");

    auto mgr = new hz_memmgr;
    mgr->set_peak(1 << 2);

    auto rose = hzcodec::white_rose("/home/supercmmetry/Documents/dickens/dickens");
    HZ_MEM_INIT_FROM(mgr, rose);

    rose.compress("/home/supercmmetry/Documents/dickens/dickens.hz");

    rose.set_file("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.decompress("/home/supercmmetry/Documents/dickens/dickens.hz.txt");

    return 0;
}