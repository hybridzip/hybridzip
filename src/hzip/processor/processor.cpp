//
// Created by supercmmetry on 7/14/20.
//

#include "processor.h"

hz_processor::hz_processor() {
    sem_init(&mutex, 0, 1);
}

void hz_processor::set_nthreads(uint64_t n) {
    sem_wait(&mutex);
    n_threads = n;
    sem_post(&mutex);
}

bool hz_processor::run(hz_job *job) {
    sem_wait(&mutex);
    if (threads_in_use == n_threads) {
        sem_post(&mutex);
        return false;
    }

    
    sem_post(&mutex);
    return true;
}

