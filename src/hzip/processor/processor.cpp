#include "processor.h"
#include <hzip/core/compressors/victini.h>

hz_processor::hz_processor(uint64_t n_threads) {
    this->n_threads = n_threads;
    this->mutex = HZ_NEW(sem_t);
    sem_init(this->mutex, 0, n_threads);
}

bool hz_processor::run(hz_job *job) {
    sem_wait(mutex);


    sem_post(mutex);
    return true;
}

hzblob_set hz_processor::hzp_split_blob(hzblob_t *blob, uint64_t size) {
    uint64_t n = blob->o_size / size;
    uint64_t r = blob->o_size % size;

    if (n == 0) {
        n = 1;
        size = 0;
    }

    auto *blobs = HZ_MALLOC(hzblob_t, n);

    for (uint64_t i = 0; i < n - 1; i++) {
        blobs[i] = hzblob_t();
        blobs[i].o_size = size;
        blobs[i].o_data = HZ_MALLOC(uint8_t, size);

        for (uint64_t j = 0; j < size; j++) {
            blobs[i].o_data[j] = blob->o_data[size * i + j];
        }
    }

    blobs[n - 1] = hzblob_t();
    blobs[n - 1].o_size = size + r;
    blobs[n - 1].o_data = HZ_MALLOC(uint8_t, size + r);

    for (uint64_t j = 0; j < size + r; j++) {
        blobs[n - 1].o_data[j] = blob->o_data[size * (n - 1) + j];
    }

    blob->destroy();
    HZ_FREE(blob);

    return hzblob_set{.blobs=blobs, .blob_count=n};
}

void hz_processor::hzp_encode(hz_job *job) {
    // 4MB blobs for victini.
    hzblob_set set = hzp_split_blob(job->blob, 0x400000);
    auto *blob_array = HZ_MALLOC(hzblob_t, set.blob_count);

    for (uint64_t i = 0; i < set.blob_count; i++) {
        auto codec = hzp_get_codec(job->algorithm);

        auto cblob = codec->compress(&set.blobs[i]);
        blob_array[i] = *cblob;

        HZ_FREE(cblob);
        set.blobs[i].destroy();

        job->archive->inject_mstate(blob_array[i].mstate, &blob_array[i]);
    }

    job->archive->create_file(job->dest, blob_array, set.blob_count);

    // cleanup
    for (uint64_t i = 0; i < set.blob_count; i++) {
        blob_array[i].destroy();
    }

    HZ_FREE(blob_array);
    HZ_FREE(set.blobs);
}

hzcodec::hz_abstract_codec *hz_processor::hzp_get_codec(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return nullptr;
        case hzcodec::algorithms::VICTINI:
            auto victini = new hzcodec::victini();
            HZ_MEM_INIT_PTR(victini);
            return victini;
    }
}

