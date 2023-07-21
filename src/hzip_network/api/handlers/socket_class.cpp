#include "socket_class.h"
#include <cerrno>
#include <cstring>
#include <utility>
#include <sys/socket.h>
#include <hzip_network/errors/api.h>
#include <hzip_network/api/api_enums.h>

using namespace hzapi;

void SocketInterface::t_send(const void *buf, size_t n) const {
    uint8_t word = COMMON_CTL_SUCCESS;
    if (send(_sock, &word, sizeof(word), 0) < sizeof(word)) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

    if (send(_sock, buf, n, 0) < n) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

//    if (errno != 0) {
//        throw ApiErrors::ConnectionError(std::string("Send operation failed: ") + strerror(errno));
//    }
}

void SocketInterface::t_recv(void *buf, size_t n, bool sync) {
    if (sync) {
        t_recv_sync();
    }

    if (recv(_sock, buf, n, MSG_WAITALL) < n) {
        throw ApiErrors::ConnectionError("Insufficient data received");
    }

//    if (errno != 0) {
//        throw ApiErrors::ConnectionError(std::string("Receive operation failed: ") + strerror(errno));
//    }
}

void SocketInterface::t_recv_sync() const {
    uint8_t word = COMMON_CTL_SUCCESS;
    if (send(_sock, &word, sizeof(word), 0) < sizeof(word)) {
        throw ApiErrors::ConnectionError("Insufficient data sent");
    }

//    if (errno != 0) {
//        throw ApiErrors::ConnectionError(std::string("Send operation failed: ") + strerror(errno));
//    }
}

void SocketInterface::error(const std::string &msg) const {
    // Error format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = COMMON_CTL_ERROR;
    send(_sock, &word, sizeof(word), 0);

    uint16_t len = msg.length();

    send(_sock, &len, sizeof(len), 0);
    send(_sock, msg.c_str(), len, 0);
}

void SocketInterface::success(const std::string &msg) {
    // Success format: <msg len (2B)> <msg (?B)>

    uint16_t len = msg.length();
    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}

SocketInterface::SocketInterface(int sock, std::string ip_addr, uint16_t port) : _sock(sock),
                                                                                 _ip_addr(std::move(ip_addr)),
                                                                                 _port(port) {

}
