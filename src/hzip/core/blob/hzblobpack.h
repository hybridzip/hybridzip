#ifndef HYBRIDZIP_HZBLOBPACK_H
#define HYBRIDZIP_HZBLOBPACK_H

#include <iostream>
#include <vector>
#include <functional>
#include <hzip/core/utils/unary.h>
#include <hzip/bitio/bitio.h>
#include "hzblob.h"


class hzBlobPacker {
private:
    std::vector<bin_t> bin_vec;
    hzrblob_set set;
public:
    hzBlobPacker(hzrblob_set _set) {
        set = _set;
    }

    void pack() {
        auto bin = unarypx_bin(set.count);
        bin_vec.push_back(bin);

        for (int i = 0; i < set.count; i++) {
            bin = unarypx_bin(set.blobs[i].size);
            bin_vec.push_back(bin);
            bin = unarypx_bin(set.blobs[i].o_size);
            bin_vec.push_back(bin);
        }

        for (int i = 0; i < set.count; i++) {
            auto blob = set.blobs[i];
            for (int j = 0; j < blob.size; j++) {
                bin_vec.push_back(bin_t{.obj=blob.data[j], .n=0x20});
            }
        }
    }

    void commit(bitio::bitio_stream stream) {
        for (auto &bin : bin_vec) {
            stream.write(bin.obj, bin.n);
        }

        stream.flush();
    }
};

class hzBlobUnpacker {
private:

public:
    hzrblob_set unpack(bitio::bitio_stream *stream) {
        // first retrieve set count.
        auto lb_stream = [stream](uint64_t n) {
            uint64_t x =  stream->read(n);
            return x;
        };

        auto set_count = unaryinv_bin(lb_stream).obj;

        hzrblob_set set;
        set.count = set_count;
        set.blobs = new hzrblob_t[set_count];

        for (int i = 0; i < set_count; i++) {
            set.blobs[i].size = unaryinv_bin(lb_stream).obj;
            set.blobs[i].o_size = unaryinv_bin(lb_stream).obj;
        }

        for (int i = 0; i < set_count; i++) {
            set.blobs[i].data = new uint64_t[set.blobs[i].size];
            for (int j = 0; j < set.blobs[i].size; j++) {
                set.blobs[i].data[j] = lb_stream(0x20);
            }
        }

        // align to next-byte.
        stream->align();

        return set;
    }
};

#endif