#ifndef HYBRIDZIP_STREAMER_H
#define HYBRIDZIP_STREAMER_H

#include <rainman/rainman.h>
#include <hzip/core/compressors/compressors.h>
#include <hzip/processor/processor.h>
#include <hzip/api/providers/archive_provider.h>
#include "socket_class.h"


namespace hzapi {
    class Streamer : public rainman::arena, public SocketInterface {
    private:
        HZ_Processor *processor;

        uint64_t hzes_b_size(hzcodec::algorithms::ALGORITHM alg);

        sem_t mutex{};

        hzapi::ArchiveProvider *archive_provider;

    public:

        Streamer(int _sock, char *_ip_addr, uint16_t port, HZ_Processor *_proc,
                 hzapi::ArchiveProvider *_archive_provider);

        void start();

        void encode();

        void decode();

        void write_mstate();

        void read_mstate();
    };
}

#endif
