#include <cerrno>
#include <loguru/loguru.hpp>
#include <sys/socket.h>
#include <signal.h>
#include <hzip/errors/api.h>
#include <hzip/api/api_enums.h>
#include <cstring>
#include "socket_class.h"

using namespace hzapi;

void hz_socket_class::t_send(const void *buf, size_t n) {
    uint8_t word = COMMON_CTL_SUCCESS;
    if (send(sock, &word, sizeof(word), 0) < sizeof(word)) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

    if (send(sock, buf, n, 0) < n) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

    if (errno != 0) {
        throw ApiErrors::ConnectionError(std::string("Send operation failed: ") + strerror(errno));
    }
}

void hz_socket_class::t_recv(void *buf, size_t n) {
    if (recv(sock, buf, n, 0) < n) {
        throw ApiErrors::ConnectionError("Insufficient data received");
    }

    if (errno != 0) {
        throw ApiErrors::ConnectionError(std::string("Receive operation failed: ") + strerror(errno));
    }

    uint8_t word = COMMON_CTL_SUCCESS;
    if (send(sock, &word, sizeof(word), 0) < sizeof(word)) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

    if (errno != 0) {
        throw ApiErrors::ConnectionError(std::string("Send operation failed: ") + strerror(errno));
    }
}

void hz_socket_class::error(const std::string &msg) {
    // Error format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = COMMON_CTL_ERROR;
    send(sock, &word, sizeof(word), 0);

    uint16_t len = msg.length();

    send(sock, &len, sizeof(len), 0);
    send(sock, msg.c_str(), len, 0);
}

void hz_socket_class::success(const std::string &msg) {
    // Success format: <msg len (2B)> <msg (?B)>

    uint16_t len = msg.length();
    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}