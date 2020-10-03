#include "processor.h"
#include <hzip/errors/processor.h>
#include <hzip/core/compressors/victini.h>

hz_processor::hz_processor(uint64_t n_threads) {
    this->n_threads = n_threads;
    sem_init(&mutex, 0, n_threads);
}

hzcodec::abstract_codec *hz_processor::hzp_get_codec(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return nullptr;
        case hzcodec::algorithms::VICTINI:
            auto victini = rnew(hzcodec::victini);
            rinitptr(victini);
            return victini;
    }
}

hzblob_set hz_processor::hzp_split(hz_codec_job *job) {
    auto *blob = job->blob;

    uint64_t size = 0;

    switch (job->algorithm) {
        case hzcodec::algorithms::UNDEFINED:
            size = blob->o_size;
            break;
        case hzcodec::algorithms::VICTINI:
            size = 0x400000;
            break;
    }

    if (blob->o_size == 0) {
        return hzblob_set{.blobs=nullptr, .blob_count=0};
    }

    uint64_t n = blob->o_size / size;
    uint64_t r = blob->o_size % size;

    if (n == 0) {
        n = 1;
        size = 0;
    }

    auto *blobs = rmalloc(hzblob_t, n);

    for (uint64_t i = 0; i < n - 1; i++) {
        blobs[i] = hzblob_t();
        blobs[i].o_size = size;
        blobs[i].o_data = rmalloc(uint8_t, size);

        for (uint64_t j = 0; j < size; j++) {
            blobs[i].o_data[j] = blob->o_data[size * i + j];
        }
    }

    blobs[n - 1] = hzblob_t();
    blobs[n - 1].o_size = size + r;
    blobs[n - 1].o_data = rmalloc(uint8_t, size + r);

    for (uint64_t j = 0; j < size + r; j++) {
        blobs[n - 1].o_data[j] = blob->o_data[size * (n - 1) + j];
    }

    blob->destroy();
    rfree(blob);

    return hzblob_set{.blobs=blobs, .blob_count=n};
}

void hz_processor::run(hz_job *job) {
    if (job == nullptr) {
        return;
    }

    if (job->codec != nullptr) {
        std::thread([this](hz_job *job) {
            try {
                sem_wait(&mutex);
                this->hzp_run_codec_job(job->codec);
                sem_post(&mutex);
            } catch (std::exception &e) {
                sem_post(&mutex);
                //todo: Send back error in callback.
            }
        }, job).detach();
    }
}

void hz_processor::hzp_encode(hz_codec_job *job) {
    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    if (job->archive == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Archive not found");
    }

    auto *blob = codec->compress(job->blob);
    auto id = job->archive->write_blob(blob);

    if (job->blob_id_callback != nullptr) {
        job->blob_id_callback(id);
    }
}

void hz_processor::hzp_run_codec_job(hz_codec_job *job) {
    switch (job->job_type) {
        case hz_codec_job::ENCODE:
            hzp_encode(job);
            break;
        case hz_codec_job::DECODE:
            hzp_decode(job);
            break;
    }
}

void hz_processor::hzp_decode(hz_codec_job *job) {
    if (job == nullptr) {
        return;
    }

    hzblob_set set = job->archive->read_file(job->dest);

    for (uint64_t i = 0; i < set.blob_count; i++) {
        auto codec = hzp_get_codec(job->algorithm);

        auto dblob = codec->decompress(&set.blobs[i]);

        set.blobs[i].destroy();
        set.blobs[i] = *dblob;

        rfree(dblob);
    }
}


// To avoid processor overload.
void hz_processor::cycle() {
    sem_wait(&mutex);
    sem_post(&mutex);
}

