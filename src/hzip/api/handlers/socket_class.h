#ifndef HYBRIDZIP_SOCKET_CLASS_H
#define HYBRIDZIP_SOCKET_CLASS_H

#include <cstdlib>
#include <cstdint>
#include <string>

#define HZ_SEND(buf, n) t_send(buf, n)
#define HZ_RECV(buf, n) t_recv(buf, n)
#define HZAPI_LOG(verbosity, str) LOG_F(verbosity, "hzip.api: [%s:%d] %s", ip_addr, port, str)


namespace hzapi {
    class hz_socket_class {
    protected:
        int sock{};
        char *ip_addr{};
        uint16_t port{};

        void t_send(const void *buf, size_t n);

        void t_recv(void *buf, size_t n);

        void error(const std::string &msg);

        void success(const std::string &msg);
    };
}

#endif
