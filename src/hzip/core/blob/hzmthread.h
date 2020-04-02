#ifndef HYBRIDZIP_HZMTHREAD_H
#define HYBRIDZIP_HZMTHREAD_H

#include <sys/types.h>
#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include <hzip/core/entropy/hzrans/hzrans.h>
#include <hzip/bitio/bitio.h>
#include <hzip/other/constants.h>
#include <hzip/utils/distribution.h>
#include "hzblob.h"
#include <hzip/core/entropy/hzrans/hzrbin.h>


void hzuGenBlob(uint64_t alpha, uint16_t scale, uint64_t size, uint64_t *dist,
                hz_codec_callback callback, std::function<uint64_t(void)> extract, hzrblob_t *targ_blob,
                hz_cross_encoder cross_encoder = nullptr) {

    auto encoder = HZRUEncoder(alpha, scale, size);
    encoder.setExtractionFunc(std::move(extract));
    encoder.setDistribution(dist);
    encoder.setCallback(std::move(callback));
    encoder.setCrossEncoder(std::move(cross_encoder));

    for (uint64_t i = 0; i < size; i++) {
        encoder.normalize();
    }

    auto dptr = encoder.encode();
    targ_blob->data = dptr.data;
    targ_blob->size = dptr.n;
    targ_blob->o_size = size;
}

void hzuDeGenBlob(hzrblob_t blob, uint64_t alpha, uint16_t scale, uint64_t *dist, hz_codec_callback _callback) {
    auto decoder = HZRUDecoder(alpha, scale, blob.o_size);
    decoder.setDistribution(dist);
    decoder.setCallback(_callback);
    decoder.decode(blob.data);
}

class HZUProcessor {
private:
    uint nthreads;
    uint64_t alphabet_size;
    uint64_t size;
    uint16_t scale;
    hz_codec_callback callback;
    std::function<uint64_t(void)> *extractors;
    hz_cross_encoder *cross_encoders;

public:
    HZUProcessor(uint n_threads) {
        nthreads = n_threads;
        set_header();
    }

    void set_header(uint64_t alphabet_size = 0x100, uint16_t scale = HZRANS_SCALE) {
        this->alphabet_size = alphabet_size;
        this->scale = scale;
    }

    void set_buffer_size(uint64_t buffer_size) {
        size = buffer_size;
    }

    void set_callback(hz_codec_callback _callback) {
        callback = _callback;
    }

    void set_extractors(std::function<uint64_t(void)> *_extractors) {
        extractors = _extractors;
    }

    void set_cross_encoders(hz_cross_encoder *_cross_encoders) {
        cross_encoders = _cross_encoders;
    }

    void use_only_base_encoder() {
        cross_encoders = new hz_cross_encoder[nthreads];
        for (int i = 0; i < nthreads; i++) {
            cross_encoders[i] = nullptr;
        }
    }

    hzrblob_set encode() {
        uint64_t block_size = size / nthreads;
        uint64_t block_residual = size % nthreads;
        std::vector<std::thread> thread_vector;
        auto *blobs = new hzrblob_t[nthreads];

        for (int i = 0; i < nthreads; i++) {
            uint64_t residual = 0;
            if (i == nthreads - 1)
                residual = block_residual;


            std::thread thread(hzuGenBlob, alphabet_size, scale, block_size + residual,
                               hzip_get_init_dist(alphabet_size), callback, extractors[i], blobs + i,
                               cross_encoders[i]);

            thread_vector.push_back(std::move(thread));
        }


        for (auto &iter : thread_vector) {
            //wait for all threads to complete encoding the blobs.
            iter.join();
        }

        return hzrblob_set{.blobs = blobs, .count = nthreads};
    }

    std::vector<uint64_t> decode(hzrblob_set set) {
        std::vector<uint64_t> union_vec;
        std::vector<std::vector<uint64_t> *> data_vectors;
        std::vector<std::thread> thread_vector;
        for (int i = 0; i < set.count; i++) {
            auto data_vector = new std::vector<uint64_t>;
            data_vectors.push_back(data_vector);
            auto _callback = this->callback;
            std::thread decoder_thread(hzuDeGenBlob, set.blobs[i], alphabet_size, scale, hzip_get_init_dist(0x100),
                                       [data_vector, _callback](uint64_t obj, uint64_t *ptr) {
                                           data_vector->push_back(obj);
                                           _callback(obj, ptr);
                                       });
            thread_vector.push_back(std::move(decoder_thread));
        }

        for (int i = 0; i < thread_vector.size(); i++) {
            // wait for thread-i and then push to union_vec.
            thread_vector[i].join();
            union_vec.resize(union_vec.size() + data_vectors[i]->size());
            std::move(data_vectors[i]->begin(), data_vectors[i]->end(),
                      union_vec.begin() + union_vec.size() - data_vectors[i]->size());

        }

        return union_vec;
    }
};


#endif
