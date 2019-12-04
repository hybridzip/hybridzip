#ifndef HYBRIDZIP_HZMTHREAD_H
#define HYBRIDZIP_HZMTHREAD_H

#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include "hzblob.h"
#include "../entropy/hzrans/hzrans.h"


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

class hzMthByteBlob {
private:
    uint32_t thread_count;
    uint64_t size;
    uint8_t *raw;
    int32_t **dist_col;
    enc_callback callback;
public:
    hzMthByteBlob(uint32_t nthreads, uint8_t *raw, uint64_t size) {
        thread_count = nthreads;
        this->raw = raw;
        this->size = size;
        dist_col = new int32_t *[nthreads];
    }

    void setCallback(enc_callback _callback) {
        this->callback = _callback;
    }

    hzrblob_set run() {
        uint64_t block_size = size / thread_count;
        uint64_t block_residual = size % thread_count;
        std::vector<std::thread> thread_vector;
        auto *blobs = new hzrblob_t[thread_count];

        for (int i = 0; i < thread_count; i++) {
            dist_col[i] = new int32_t[0x100];
            std::thread thread(hzGenBlob, raw + i * block_size, block_size, dist_col[i], callback, blobs + i);
            thread_vector.push_back(std::move(thread));
        }
        for(auto & iter : thread_vector) {
            //wait for all threads to complete encoding the blobs.
            iter.join();
        }

        return hzrblob_set {.blobs = blobs, .count = thread_count};
    }
};


#endif
