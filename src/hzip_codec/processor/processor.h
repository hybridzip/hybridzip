#ifndef HYBRIDZIP_PROCESSOR_H
#define HYBRIDZIP_PROCESSOR_H

#include <thread>
#include <semaphore>
#include <rainman/rainman.h>
#include <hzip_codec/compressors.h>
#include "job.h"

// This is an abstract processor for hybridzip.
// todo: Add CUDA co-processor

class HZ_Processor : private rainman::Allocator {
private:
    uint64_t n_threads{};
    uint64_t threads_in_use{};
    std::counting_semaphore<> _semaphore{};

    hzcodec::AbstractCodec *hzp_get_codec(hzcodec::algorithms::ALGORITHM alg);

    void hzp_encode(const rainman::ptr<HZ_CodecJob> &job);

    void hzp_decode(const rainman::ptr<HZ_CodecJob> &job);

    void hzp_train(const rainman::ptr<HZ_CodecJob> &job);

    void hzp_run_codec_job(const rainman::ptr<HZ_CodecJob> &job);

public:
    HZ_Processor() = default;

    HZ_Processor(uint64_t n_threads);

    void run(const rainman::ptr<HZ_Job> &job);

    void cycle();
};


#endif
