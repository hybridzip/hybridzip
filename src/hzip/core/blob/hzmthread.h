#ifndef HYBRIDZIP_HZMTHREAD_H
#define HYBRIDZIP_HZMTHREAD_H

#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include <hzip/core/entropy/hzrans/hzrans.h>
#include <hzip/bitio/bitio.h>
#include <hzip/other/constants.h>
#include <hzip/utils/distribution.h>
#include "hzblob.h"
#include <hzip/core/entropy/hzrans/hzrbyte.h>
#include <hzip/core/entropy/hzrans/hzrbin.h>

void hzGenBlob(uint8_t *raw, uint64_t size, uint64_t *dist, hz_codec_callback callback, hzrblob_t *targ_blob) {
    hzrByteEncoder hbenc(size, HZRANS_SCALE);
    hbenc.setDistribution(dist);
    for (auto i = 0; i < size; i++) {
        hbenc.normalize(raw[i], callback);
    }
    auto dptr = hbenc.encodeBytes();
    targ_blob->data = dptr.data;
    targ_blob->size = dptr.n;
    targ_blob->o_size = size;
}

void hzDeGenBlob(hzrblob_t blob, uint64_t *dist, hz_codec_callback _callback) {
    auto hbdec = hzrByteDecoder(blob.o_size, HZRANS_SCALE);
    hbdec.setDistribution(dist);
    hbdec.decodeBytes(blob.data, _callback);
}

void hzuGenBlob(uint64_t alpha, uint16_t scale, uint64_t size, uint64_t *dist,
                hz_codec_callback callback, std::function<uint64_t(void)> extract, hzrblob_t *targ_blob) {

    auto encoder = HZRUEncoder(alpha, scale, size);
    encoder.setExtractionFunc(std::move(extract));
    encoder.setDistribution(dist);
    encoder.setCallback(callback);
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

class hzMultiByteBlobProcessor {
private:
    uint64_t thread_count;
    uint64_t size;
    hz_codec_callback callback;
public:
    hzMultiByteBlobProcessor(uint64_t nthreads, uint64_t size) {
        thread_count = nthreads;
        this->size = size;
    }

    hzMultiByteBlobProcessor() {
        thread_count = 0;
        this->size = 0;
    }

    void setCallback(hz_codec_callback _callback) {
        this->callback = _callback;
    }

    void setSize(uint64_t size) {
        this->size = size;
    }

    hzrblob_set run_encoder(uint8_t *raw) {
        uint64_t block_size = size / thread_count;

        if (block_size == 0) {
            thread_count = 1;
            block_size = size;
        }

        uint64_t block_residual = size % thread_count;
        std::vector<std::thread> thread_vector;
        auto *blobs = new hzrblob_t[thread_count];

        for (int i = 0; i < thread_count; i++) {
            uint64_t residual = 0;
            if (i == thread_count - 1)
                residual = block_residual;

            auto _callback = this->callback;

            std::thread thread(hzGenBlob, raw + i * block_size, block_size + residual, hzip_get_init_dist(0x100),
                               _callback,
                               blobs + i);
            thread_vector.push_back(std::move(thread));
        }


        for (auto &iter : thread_vector) {
            //wait for all threads to complete encoding the blobs.
            iter.join();
        }

        return hzrblob_set{.blobs = blobs, .count = thread_count};
    }

    void run_decoder(hzrblob_set set, char *filename) {
        std::vector<bitio::bitio_byte_dumper *> dumpers;
        std::vector<std::thread> thread_vector;

        for (int i = 0; i < set.count; i++) {
            auto dumper = new bitio::bitio_byte_dumper(filename, true);
            dumpers.push_back(dumper);
            auto _callback = this->callback;
            std::thread decoder_thread(hzDeGenBlob, set.blobs[i], hzip_get_init_dist(0x100),
                                       [dumper, _callback](uint64_t obj, uint64_t *ptr) {
                                           dumper->write_byte(obj);
                                           _callback(obj, ptr);
                                       });
            thread_vector.push_back(std::move(decoder_thread));
        }

        for (int i = 0; i < thread_vector.size(); i++) {
            // wait for thread-i and then dump bytes to file.
            thread_vector[i].join();
            dumpers[i]->dump();
        }

    }

    std::vector<uint8_t *> run_decoder(hzrblob_set set) {
        std::vector<bitio::bitio_byte_dumper *> dumpers;
        std::vector<std::thread> thread_vector;
        for (int i = 0; i < set.count; i++) {
            auto dumper = new bitio::bitio_byte_dumper(nullptr);
            dumpers.push_back(dumper);
            auto _callback = this->callback;
            std::thread decoder_thread(hzDeGenBlob, set.blobs[i], hzip_get_init_dist(0x100),
                                       [dumper, _callback](uint64_t obj, uint64_t *ptr) {
                                           dumper->write_byte(obj);
                                           _callback(obj, ptr);
                                       });
            thread_vector.push_back(std::move(decoder_thread));
        }

        std::vector<uint8_t *> byte_set;
        for (int i = 0; i < thread_vector.size(); i++) {
            // wait for thread-i and then push to byte-set.
            thread_vector[i].join();
            byte_set.push_back(dumpers[i]->get_bytes());
        }

        return byte_set;
    }
};

class HZUProcessor {
private:
    uint nthreads;
    uint64_t alphabet_size;
    uint64_t size;
    uint16_t scale;
    hz_codec_callback callback;
    std::function<uint64_t(void)> *extractors;

public:
    HZUProcessor(uint n_threads) {
        nthreads = n_threads;
        setHeader();
    }

    void setHeader(uint64_t alphabet_size = 0x100, uint16_t scale = HZRANS_SCALE) {
        this->alphabet_size = alphabet_size;
        this->scale = scale;
    }

    void setBufferSize(uint64_t buffer_size) {
        size = buffer_size;
    }

    void setCallback(hz_codec_callback _callback) {
        callback = _callback;
    }

    void setExtractors(std::function<uint64_t(void)> *_extractors) {
        extractors = _extractors;
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
                               hzip_get_init_dist(alphabet_size), callback, extractors[i], blobs + i);
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
