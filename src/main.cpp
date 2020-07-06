#include <iostream>
#include <hzip/core/compressors/white_rose.h>
#include <hzip/memory/memmgr.h>

int main() {
    std::cout << "hybridzip - v1.0.0 (jpeg-dev)" << std::endl;

//    auto rose = hzcodec::white_rose("/home/supercmmetry/Documents/dickens/dickens");
//    rose.compress("/home/supercmmetry/Documents/dickens/dickens.hz");
//    rose.set_file("/home/supercmmetry/Documents/dickens/dickens.hz");
//    rose.decompress("/home/supercmmetry/Documents/dickens/dickens.hz.txt");

    hz_memmgr memmgr;

    int *x = memmgr.hz_malloc<int>(100);

    for (int i = 0; i < 100; i++) {
        x[i] = i + 1;
        char *y = memmgr.hz_malloc<char>(i);
        memmgr.hz_free(y);
    }

    memmgr.hz_free(x);

    std::cout << memmgr.allocation_size << " - " << memmgr.n_allocations << std::endl;

    return 0;
}