#ifndef HYBRIDZIP_STREAM_H
#define HYBRIDZIP_STREAM_H

#include <rainman/rainman.h>
#include <hzip/core/compressors/compressors.h>
#include <hzip/processor/processor.h>
#include <hzip/api/providers/archive_provider.h>
#include "socket_class.h"


namespace hzapi {
    class hz_streamer : public rainman::module, public hz_socket_class {
    private:
        hz_processor *processor;

        uint64_t hzes_b_size(hzcodec::algorithms::ALGORITHM alg);

        sem_t mutex{};

        hzprovider::archive *archive_provider;

    public:

        hz_streamer(int _sock, char *_ip_addr, uint16_t port, hz_processor *_proc,
                    hzprovider::archive *_archive_provider);

        void start();

        void encode();

        void decode();

        void write_mstate();

        void read_mstate();
    };
}

#endif
