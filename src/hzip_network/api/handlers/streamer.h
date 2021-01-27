#ifndef HYBRIDZIP_STREAMER_H
#define HYBRIDZIP_STREAMER_H

#include <rainman/rainman.h>
#include <hzip_core/core/compressors/compressor_enums.h>
#include <hzip_core/core/compressors/compressors.h>
#include <hzip_core/processor/processor.h>
#include <hzip_network/api/providers/archive_provider.h>
#include "socket_class.h"


namespace hzapi {
    class Streamer : public SocketInterface {
    private:
        rainman::ptr<HZ_Processor> _processor;
        rainman::ptr<std::mutex> _mutex;
        rainman::ptr<hzapi::ArchiveProvider> _archive_provider;

        static uint64_t hzes_b_size(hzcodec::algorithms::ALGORITHM alg);
    public:

        Streamer(int sock, const std::string &ip_addr, uint16_t port, const rainman::ptr<HZ_Processor>& processor,
                 const rainman::ptr<hzapi::ArchiveProvider>& archive_provider);

        void start();

        void encode();

        void decode();

        void write_mstate();

        void read_mstate();
    };
}

#endif
