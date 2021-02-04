#ifndef HYBRIDZIP_API_QUERY_H
#define HYBRIDZIP_API_QUERY_H

#include <rainman/rainman.h>
#include <hzip_codec/processor/processor.h>
#include <hzip_network/api/providers/archive_provider.h>
#include "socket_class.h"

namespace hzapi {
    class Query : public SocketInterface {
    private:
        rainman::ptr<hzapi::ArchiveProvider> _archive_provider{};
    public:
        Query(int sock, const std::string &ip_addr, uint16_t port, const rainman::ptr<hzapi::ArchiveProvider> &archive_provider);

        void start();
    };
}

#endif
