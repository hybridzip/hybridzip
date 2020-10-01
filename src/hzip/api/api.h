#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <rainman/rainman.h>
#include <hzip/processor/processor.h>

class hz_api_instance : rainman::context {
private:
    hz_processor *processor{};

public:
    hz_api_instance(hz_processor *_processor);
};

// An API based on socket-communication.
class hz_api : rainman::context {
private:
    hz_processor *processor{};
    uint64_t max_instances = 1;

public:
    hz_api *limit(uint64_t _max_instances);

    hz_api *process(uint64_t _n_threads);

    hz_api *init();
};

#endif
