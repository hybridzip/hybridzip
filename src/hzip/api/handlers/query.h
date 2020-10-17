#ifndef HYBRIDZIP_API_QUERY_H
#define HYBRIDZIP_API_QUERY_H

#include <rainman/rainman.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/providers/archive_provider.h>
#include <hzip/processor/processor.h>

namespace hzapi {
    class hz_query : public rainman::module, public hz_socket_class {
    private:
        hzprovider::archive *archive_provider{};
    public:
        hz_query(int _sock, char *_ip_addr, uint16_t port, hzprovider::archive *_archive_provider);

        void start();
    };
}

#endif
