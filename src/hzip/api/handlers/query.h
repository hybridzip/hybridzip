#ifndef HYBRIDZIP_API_QUERY_H
#define HYBRIDZIP_API_QUERY_H

#include <rainman/rainman.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/providers/archive_provider.h>
#include <hzip/processor/processor.h>

namespace hzapi {
    class Query : public rainman::arena, public SocketInterface {
    private:
        hzapi::ArchiveProvider *archive_provider{};
    public:
        Query(int _sock, char *_ip_addr, uint16_t port, hzapi::ArchiveProvider *_archive_provider);

        void start();
    };
}

#endif
