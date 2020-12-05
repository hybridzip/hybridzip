#ifndef HYBRIDZIP_SOCKET_CLASS_H
#define HYBRIDZIP_SOCKET_CLASS_H

#include <cstdlib>
#include <cstdint>
#include <string>

#define HZ_SEND(buf, n) t_send(buf, n)
#define HZ_RECV(buf, n) t_recv(buf, n)
#define HZ_RECV_SYNC t_recv_sync()
#define HZAPI_LOG(verbosity, str) LOG_F(verbosity, "hzip.api: [%s:%d] %s", ip_addr, port, str)

#define HZAPI_LOGF(verbosity, fmt, ...) [&]() {                 \
    auto s = "hzip.api: [%s:%d] " + std::string(fmt);           \
    LOG_F(verbosity, s.c_str(), ip_addr, port, __VA_ARGS__);    \
}()                                                             \

namespace hzapi {
    class SocketInterface {
    protected:
        int sock{};
        char *ip_addr{};
        uint16_t port{};

        void t_send(const void *buf, size_t n);

        void t_recv(void *buf, size_t n, bool sync = true);

        void t_recv_sync() const;

        void error(const std::string &msg);

        void success(const std::string &msg);
    };
}

#endif
