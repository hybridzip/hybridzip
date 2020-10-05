#ifndef HYBRIDZIP_STREAM_H
#define HYBRIDZIP_STREAM_H

#include <rainman/rainman.h>
#include <hzip/core/compressors/compressors.h>
#include <hzip/processor/processor.h>
#include <hzip/api/providers/archive_provider.h>
#include "socket_class.h"

class hz_streamer : public rainman::module, public hz_socket_class {
private:
    hz_processor *processor;
    uint64_t hzes_b_size(hzcodec::algorithms::ALGORITHM alg);
    sem_t mutex{};

    hzprovider::archive *archive_provider;

    enum STREAM_CTL {
        STREAM_CTL_ENCODE = 0x0,
        STREAM_CTL_DECODE = 0x1,
    };

    enum ENCODE_CTL {
        ENCODE_CTL_STREAM = 0x0,
        ENCODE_CTL_MSTATE_ADDR = 0x1,
        ENCODE_CTL_ARCHIVE = 0x2,
        ENCODE_CTL_DEST = 0x3,
        ENCODE_CTL_ALGORITHM = 0x4,
        ENCODE_CTL_PIGGYBACK = 0x5,
    };

    enum DECODE_CTL {
        DECODE_CTL_STREAM = 0x0,
        DECODE_CTL_MSTATE_ADDR = 0x1,
        DECODE_CTL_ARCHIVE = 0x2,
        DECODE_CTL_SRC = 0x3,
        DECODE_CTL_ALGORITHM = 0x4,
        DECODE_CTL_PIGGYBACK = 0x5,
        DECODE_CTL_MSTATE_STREAM = 0x6,
        DECODE_CTL_BLOB_STREAM = 0x7,
    };
public:
    hz_streamer(int _sock, char *_ip_addr, uint16_t port, hz_processor *_proc, hzprovider::archive *_archive_provider);

    void start();

    void encode();

    void decode();
};

#endif
