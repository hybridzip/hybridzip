#ifndef HYBRIDZIP_HZMTHREAD_H
#define HYBRIDZIP_HZMTHREAD_H

#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include "hzblob.h"
#include "../entropy/hzrans/hzrans.h"
#include "../../bitio/bitio.h"
#include "../../other/constants.h"
#include "../utils/distribution.h"

void hzGenBlob(uint8_t *raw, uint64_t size, int32_t *dist, enc_callback callback, hzrblob_t *targ_blob) {
    hzrByteEncoder hbenc(size, HZRANS_SCALE);
    for (int i = 0; i < 0x100; i++) dist[i] = 1;
    hbenc.setDistribution(dist);
    for (auto i = 0; i < size; i++) {
        hbenc.normalize(raw[i], callback);
    }
    auto dptr = hbenc.encodeBytes();
    targ_blob->data = dptr.data;
    targ_blob->size = dptr.n;
}

void hzDeGenBlob(hzrblob_t blob, int32_t *dist, std::function<void(uint8_t)> _callback) {
    auto hbdec = hzrByteDecoder(blob.size, HZRANS_SCALE);
    hbdec.setDistribution(dist);
    hbdec.decodeBytes(blob.data, _callback);
}

class hzMultiByteBlobProcessor {
private:
    uint32_t thread_count;
    uint64_t size;
    int32_t **dist_col;
    enc_callback callback;
public:
    hzMultiByteBlobProcessor(uint32_t nthreads, uint64_t size) {
        thread_count = nthreads;
        this->size = size;
        dist_col = new int32_t *[nthreads];
    }

    void setCallback(enc_callback _callback) {
        this->callback = _callback;
    }

    hzrblob_set run_encoder(uint8_t *raw) {
        uint64_t block_size = size / thread_count;
        uint64_t block_residual = size % thread_count;
        std::vector<std::thread> thread_vector;
        auto *blobs = new hzrblob_t[thread_count];

        for (int i = 0; i < thread_count; i++) {
            dist_col[i] = new int32_t[0x100];
            uint64_t residual = 0;
            if (i == thread_count - 1)
                residual = block_residual;
            std::thread thread(hzGenBlob, raw + i * block_size, block_size + residual, dist_col[i], callback,
                               blobs + i);
            thread_vector.push_back(std::move(thread));
        }


        for (auto &iter : thread_vector) {
            //wait for all threads to complete encoding the blobs.
            iter.join();
        }

        return hzrblob_set{.blobs = blobs, .count = thread_count};
    }

    void run_decoder(hzrblob_set set, char* filename) {
        std::vector<bitio::bitio_byte_dumper*> dumpers;
        std::vector<std::thread> thread_vector;
        for(int i = 0; i < set.count; i++) {
            bitio::bitio_byte_dumper dumper(filename);
            dumpers.push_back(&dumper);
            std::thread decoder_thread(hzDeGenBlob, set.blobs[i], hzip_get_init_dist(), [&dumper](uint8_t byte) {dumper.write_byte(byte);});
            thread_vector.push_back(std::move(decoder_thread));
        }

        for(int i = 0; i < thread_vector.size(); i++) {
            // wait for thread-i and then dump bytes to file.
            thread_vector[i].join();
            dumpers[i]->dump();
        }
    }

    std::vector<uint8_t*> run_decoder(hzrblob_set set) {
        std::vector<bitio::bitio_byte_dumper*> dumpers;
        std::vector<std::thread> thread_vector;
        for(int i = 0; i < set.count; i++) {
            bitio::bitio_byte_dumper dumper(nullptr);
            dumpers.push_back(&dumper);
            std::thread decoder_thread(hzDeGenBlob, set.blobs[i], hzip_get_init_dist(), [&dumper](uint8_t byte) {dumper.write_byte(byte);});
            thread_vector.push_back(std::move(decoder_thread));
        }

        std::vector<uint8_t*> byte_set;
        for(int i = 0; i < thread_vector.size(); i++) {
            // wait for thread-i and then push to byte-set.
            thread_vector[i].join();
            byte_set.push_back(dumpers[i]->get_bytes());
        }

        return byte_set;
    }
};


#endif
