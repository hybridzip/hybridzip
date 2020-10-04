#ifndef HYBRIDZIP_SOCKET_CLASS_H
#define HYBRIDZIP_SOCKET_CLASS_H

#include <cstdlib>
#include <cstdint>
#include <string>

#define HZ_SEND(buf, n) if (t_send(buf, n)) return
#define HZ_RECV(buf, n) if (t_recv(buf, n)) return

enum API_CTL {
    API_CTL_SUCCESS = 0x0,
    API_CTL_PIGGYBACK = 0x1,
    API_CTL_ERROR = 0xff,
};

class hz_socket_class {
protected:
    int sock{};
    char *ip_addr{};
    uint16_t port{};

    bool t_send(const void *buf, size_t n);

    bool t_recv(void *buf, size_t n);

    void error(const std::string &msg);

    void success(const std::string &msg);
};

#endif
