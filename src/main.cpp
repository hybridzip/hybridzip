#include <iostream>
#include <hzip/core/compressors/white_rose.h>
#include <hzip/memory/memmgr.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (memory-manager)" << std::endl;

    auto mgr = new hz_memmgr;

    auto rose = hzcodec::white_rose("/home/supercmmetry/Documents/dickens/dickens");
    rose.attach_memmgr(mgr);

    rose.compress("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.set_file("/home/supercmmetry/Documents/dickens/dickens.hz");
    rose.decompress("/home/supercmmetry/Documents/dickens/dickens.hz.txt");

    std::cout << "residual memory: " << mgr->allocation_size << " bytes" << std::endl;

    return 0;
}