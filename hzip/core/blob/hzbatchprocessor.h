#ifndef HYBRIDZIP_HZBATCHPROCESSOR_H
#define HYBRIDZIP_HZBATCHPROCESSOR_H

#include <cstdio>
#include <cstdint>
#include <functional>
#include "hzmthread.h"
#include "hzblobpack.h"
#include "../../bitio/bitio.h"

#ifndef HZ_BITIO_BUFFER_SIZE
#define HZ_BITIO_BUFFER_SIZE 0x400
#endif

namespace fs = boost::filesystem;

class hzBatchProcessor {
private:
    char *in, *out;
    uint64_t size;
    uint64_t threads_per_batch;
public:
    hzBatchProcessor(uint64_t batch_size, uint64_t threads_per_batch, char *in, char *out) {
        this->in = in;
        this->out = out;
        size = batch_size;
        this->threads_per_batch = threads_per_batch;
    };

    void compress_batch(std::function<void(uint8_t, int32_t *)> callback) {
        hzMultiByteBlobProcessor hzmbb(threads_per_batch, size);
        hzmbb.setCallback(std::move(callback));
        auto bstream = bitio::bitio_stream(out, bitio::WRITE, HZ_BITIO_BUFFER_SIZE);


        int64_t file_size = fs::file_size(fs::path{in});
        int64_t rem = file_size;
        FILE *fp = fopen(in, "rb");

        while (rem > 0) {
            char *buffer;
            if (rem > size) {
                auto buf = new char[size];
                fread(buf, 1, size,fp);
                buffer = buf;
            } else {
                auto buf = new char[rem];
                fread(buf, 1, rem,fp);
                buffer = buf;
            }
            auto set = hzmbb.run_encoder((uint8_t*)buffer);
            hzBlobPacker packer(set);
            packer.pack();
            packer.commit(bstream);
            rem -= size;
        }
    }
};

#endif
