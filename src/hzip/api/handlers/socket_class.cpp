#include <cerrno>
#include <loguru/loguru.hpp>
#include <sys/socket.h>
#include <hzip/errors/api.h>
#include <hzip/api/api_enums.h>
#include "socket_class.h"

using namespace hzapi;

void hz_socket_class::t_send(const void *buf, size_t n) {
    if (send(sock, buf, n, 0) < n) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

    if (errno != 0) {
        LOG_F(ERROR, "hzip.api: send() failed for %s:%d with errno=%d", ip_addr, port, errno);
        throw ApiErrors::ConnectionError("Send operation failed");
    }
}

void hz_socket_class::t_recv(void *buf, size_t n) {
    if (recv(sock, buf, n, 0) < n) {
        throw ApiErrors::ConnectionError("Insufficient data received");
    }

    if (errno != 0) {
        LOG_F(ERROR, "hzip.api: recv() failed for %s:%d with errno=%d", ip_addr, port, errno);
        throw ApiErrors::ConnectionError("Receive operation failed");
    }
}

void hz_socket_class::error(const std::string &msg) {
    // Error format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = COMMON_CTL_ERROR;
    HZ_SEND(&word, sizeof(word));

    uint64_t len = msg.length();

    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}

void hz_socket_class::success(const std::string &msg) {
    // Success format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = COMMON_CTL_SUCCESS;

    HZ_SEND(&word, sizeof(word));

    uint64_t len = msg.length();
    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}
