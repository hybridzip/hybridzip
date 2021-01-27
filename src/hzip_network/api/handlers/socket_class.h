#ifndef HYBRIDZIP_SOCKET_CLASS_H
#define HYBRIDZIP_SOCKET_CLASS_H

#include <cstdlib>
#include <cstdint>
#include <string>
#include <loguru/loguru.hpp>

#define HZ_SEND(buf, n) t_send(buf, n)
#define HZ_RECV(buf, n) t_recv(buf, n)
#define HZ_RECV_SYNC t_recv_sync()
#define HZAPI_LOG(verbosity, str) LOG_F(verbosity, "hzip_network.api: [%s:%d] %s", _ip_addr.c_str(), _port, str)

#define HZAPI_LOGF(verbosity, fmt, ...) [&]() {                 \
    auto s = "hzip_network.api: [%s:%d] " + std::string(fmt);           \
    LOG_F(verbosity, s.c_str(), _ip_addr.c_str(), _port, __VA_ARGS__);    \
}()                                                             \

namespace hzapi {
    class SocketInterface {
    protected:
        int _sock{};
        std::string _ip_addr{};
        uint16_t _port{};

        void t_send(const void *buf, size_t n) const;

        void t_recv(void *buf, size_t n, bool sync = true);

        void t_recv_sync() const;

        void error(const std::string &msg) const;

        void success(const std::string &msg);
    };
}

#endif
