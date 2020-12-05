#ifndef HYBRIDZIP_API_QUERY_H
#define HYBRIDZIP_API_QUERY_H

#include <rainman/rainman.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/providers/archive_provider.h>
#include <hzip/processor/processor.h>

namespace hzapi {
    class Query : public rainman::module, public SocketInterface {
    private:
        hzprovider::ArchiveProvider *archive_provider{};
    public:
        Query(int _sock, char *_ip_addr, uint16_t port, hzprovider::ArchiveProvider *_archive_provider);

        void start();
    };
}

#endif
