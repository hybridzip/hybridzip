#ifndef HYBRIDZIP_PROCESSOR_H
#define HYBRIDZIP_PROCESSOR_H

#include <thread>
#include <semaphore.h>
#include <hzip/memory/mem_interface.h>
#include "job.h"

// This is an abstract processor for hybridzip.
// todo: Add CUDA co-processor

class hz_processor: public hz_mem_iface {
private:
    uint64_t n_threads;
    uint64_t threads_in_use;
    sem_t mutex;
public:
    hz_processor();

    void set_nthreads(uint64_t n);

    bool run(hz_job *job);
};


#endif
