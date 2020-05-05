#ifndef HYBRIDZIP_HZBLOBPACK_H
#define HYBRIDZIP_HZBLOBPACK_H

#include "hzblob.h"
#include <hzip/utils/utils.h>
#include <bitio/bitio.h>
#include <iostream>
#include <vector>
#include <functional>



class hz_blob_packer {
private:
    std::vector<bin_t> bin_vec;

public:
    void pack_header(bin_t bin);

    void pack(hzrblob_set set);

    void commit(bitio::bitio_stream stream);
};

class hz_blob_unpacker {
private:
    bitio::bitio_stream *stream;
public:
    hz_blob_unpacker(bitio::bitio_stream *stream);

    HZ_SIZE_T unpack_header(HZ_UINT n);

    hzrblob_set unpack();
};

#endif
