#include <iostream>
#include <hzip/core/compressors/white_rose.h>
#include <hzip/memory/memmgr.h>
#include <loguru/loguru.hpp>

void setup_logger(int argc, char **argv) {
    loguru::g_preamble_file = false;
    loguru::init(argc, argv);
}

int main(int argc, char **argv) {
    setup_logger(argc, argv);

    LOG_F(INFO, "hzip: Started hybridzip server");

    auto mgr = new hz_memmgr;
    mgr->set_peak(1 << 30);

    auto rose = hzcodec::white_rose("/home/supercmmetry/Documents/dickens/dickens");
    HZ_MEM_INIT_FROM(mgr, rose);

    rose.compress("/home/supercmmetry/Documents/dickens/dickens.hz");

    rose.set_file("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.decompress("/home/supercmmetry/Documents/dickens/dickens.hz.txt");

    return 0;
}