#ifndef HYBRIDZIP_PROCESSOR_H
#define HYBRIDZIP_PROCESSOR_H

#include <thread>
#include <semaphore.h>
#include <rainman/rainman.h>
#include <hzip/core/compressors/compressor_base.h>
#include "job.h"

// This is an abstract processor for hybridzip.
// todo: Add CUDA co-processor

class HZ_Processor : public rainman::context {
private:
    uint64_t n_threads{};
    uint64_t threads_in_use{};
    sem_t mutex{};

    hzcodec::AbstractCodec *hzp_get_codec(hzcodec::algorithms::ALGORITHM alg);

    void hzp_encode(HZ_CodecJob *job);

    void hzp_decode(HZ_CodecJob *job);

    void hzp_train(HZ_CodecJob *job);

    void hzp_run_codec_job(HZ_CodecJob *job);

public:
    HZ_Processor() = default;

    HZ_Processor(uint64_t n_threads);

    void run(HZ_Job *job);

    void cycle();
};


#endif
