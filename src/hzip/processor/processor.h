#ifndef HYBRIDZIP_PROCESSOR_H
#define HYBRIDZIP_PROCESSOR_H

#include <thread>
#include <semaphore.h>
#include <rainman/rainman.h>
#include <hzip/core/compressors/compressor_base.h>
#include "job.h"

// This is an abstract processor for hybridzip.
// todo: Add CUDA co-processor

class hz_processor: public rainman::context {
private:
    uint64_t n_threads;
    uint64_t threads_in_use;
    sem_t mutex{};

    hzblob_set hzp_split(hz_codec_job *job);

    hzcodec::abstract_codec *hzp_get_codec(hzcodec::algorithms::ALGORITHM alg);

    void hzp_encode(hz_codec_job *job);

    void hzp_decode(hz_codec_job *job);

    void hzp_run_codec_job(hz_codec_job *job);

public:
    hz_processor() = default;

    hz_processor(uint64_t n_threads);

    void run(hz_job *job);
};


#endif
