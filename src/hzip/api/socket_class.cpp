#include <cerrno>
#include <loguru/loguru.hpp>
#include <sys/socket.h>
#include "socket_class.h"

bool hz_socket_class::t_send(const void *buf, size_t n) {
    if (send(sock, buf, n, 0) < n) {
        return true;
    }

    if (errno != 0) {
        LOG_F(ERROR, "hzip.api: send() failed for %s:%d with errno=%d", ip_addr, port, errno);
        return true;
    }

    return false;
}

bool hz_socket_class::t_recv(void *buf, size_t n) {
    if (recv(sock, buf, n, 0) < n) {
        return true;
    }

    if (errno != 0) {
        LOG_F(ERROR, "hzip.api: recv() failed for %s:%d with errno=%d", ip_addr, port, errno);
        return true;
    }

    return false;
}
