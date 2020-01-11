#ifndef HYBRIDZIP_HZBATCHPROCESSOR_H
#define HYBRIDZIP_HZBATCHPROCESSOR_H

#include <cstdio>
#include <cstdint>
#include <functional>
#include <cassert>
#include <hzip/bitio/bitio.h>
#include <hzip/utils/boost_utils.h>
#include "hzmthread.h"
#include "hzblobpack.h"





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

    void set_src_dest(char *in,char *out) {
        this->in = in;
        this->out = out;
    }

    void compress_batch(std::function<void(uint8_t, uint64_t *)> callback) {
        hzMultiByteBlobProcessor hzmbb(threads_per_batch, size);
        hzmbb.setCallback(std::move(callback));

        hzboost::deleteFileIfExists(out);
        auto bstream = bitio::bitio_stream(out, bitio::APPEND, HZ_BITIO_BUFFER_SIZE);


        int64_t file_size = hzboost::getFileSize(in);
        int64_t rem = file_size;
        FILE *fp = fopen(in, "rb");

        while (rem > 0) {
            char *buffer;
            if (rem > size) {
                auto buf = new char[size];
                fread(buf, 1, size, fp);
                buffer = buf;
            } else {
                auto buf = new char[rem];
                fread(buf, 1, rem, fp);
                buffer = buf;
                hzmbb.setSize(rem);
            }
            auto set = hzmbb.run_encoder((uint8_t *) buffer);
            hzBlobPacker packer(set);
            packer.pack();
            packer.commit(bstream);
            rem -= size;
        }
        bstream.close();
    }

    void decompress_batch(std::function<void(uint8_t, uint64_t *)> callback) {
        auto hzmbb = hzMultiByteBlobProcessor(threads_per_batch, size);
        hzmbb.setCallback(callback);
        hzboost::deleteFileIfExists(out);

        auto rstream = bitio::bitio_stream(in, bitio::READ, HZ_BITIO_BUFFER_SIZE);


        auto unpacker = hzBlobUnpacker();

        while(!rstream.isEOF()) {
            auto blobset = unpacker.unpack(&rstream);
            hzmbb.run_decoder(blobset, out);
        }
    }
};

#endif
