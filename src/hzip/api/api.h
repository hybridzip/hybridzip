#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <vector>
#include <thread>
#include <semaphore.h>
#include <rainman/rainman.h>
#include <hzip/processor/processor.h>

class hz_api_instance : rainman::context {
private:
    hz_processor *processor{};
    int sock{};
    std::thread thread{};

public:
    hz_api_instance() = default;

    hz_api_instance(int _sock, hz_processor *_processor, sem_t *mutex);

    void handshake();
};

// An API based on socket-communication.
class hz_api : rainman::context {
private:
    hz_processor *processor{};
    uint64_t max_instances = 1;
    sem_t *mutex{};

public:
    hz_api *limit(uint64_t _max_instances);

    hz_api *process(uint64_t _n_threads);

    [[noreturn]] void start(const char *addr, uint16_t port);
};

#endif
