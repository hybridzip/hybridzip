#ifndef HYBRIDZIP_STREAM_H
#define HYBRIDZIP_STREAM_H

#include <rainman/rainman.h>
#include <hzip/core/compressors/compressors.h>
#include <hzip/processor/processor.h>
#include "socket_class.h"

class hz_encode_stream : public rainman::module, public hz_socket_class {
private:
    hz_processor *proc;
    uint64_t hzes_b_size(hzcodec::algorithms::ALGORITHM alg);
    sem_t mutex{};

public:
    hz_encode_stream(int _sock, char *_ip_addr, uint16_t port, hz_processor *_proc);

    void start();
};

#endif
