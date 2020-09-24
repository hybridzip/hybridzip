#ifndef HYBRIDZIP_PROCESSOR_H
#define HYBRIDZIP_PROCESSOR_H

#include <thread>
#include <semaphore.h>
#include <hzip/memory/mem_interface.h>
#include <hzip/core/compressors/compressor_base.h>
#include "job.h"

// This is an abstract processor for hybridzip.
// todo: Add CUDA co-processor

class hz_processor: public hz_mem_iface {
private:
    uint64_t n_threads;
    uint64_t threads_in_use;
    sem_t *mutex;

    hzblob_set hzp_split_blob(hzblob_t *blob, uint64_t size);

    hzcodec::hz_abstract_codec *hzp_get_codec(hzcodec::algorithms::ALGORITHM alg);

    void hzp_encode(hz_job *job);

public:
    hz_processor(uint64_t n_threads);

    bool run(hz_job *job);
};


#endif
