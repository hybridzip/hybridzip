#include "api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hzip/errors/api.h>
#include <hzip/utils/utils.h>

hz_api_instance::hz_api_instance(int _sock, hz_processor *_processor, const std::string &_passwd, sem_t *mutex) {
    processor = _processor;
    sock = _sock;
    passwd = _passwd;
    thread = std::thread([this, mutex]() {
        sem_wait(mutex);
        if (!handshake()) {
            sem_post(mutex);
        }


        sem_post(mutex);
    });
}

bool hz_api_instance::handshake() {
    uint64_t token = hz_rand64();
    uint64_t xtoken = hz_enc_token(passwd, token);

    send(sock, &xtoken, sizeof(token), 0);
    recv(sock, &xtoken, sizeof(xtoken), 0);

    if (token == xtoken) {
        success("Handshake was successful.");
        return true;
    } else {
        error("Handshake was not successful.");
        return false;
    }
}

void hz_api_instance::error(const std::string &msg) {
    // Error format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = CTL_ERROR;
    send(sock, &word, sizeof(word), 0);

    uint64_t len = msg.length();
    send(sock, &len, sizeof(len), 0);

    send(sock, msg.c_str(), len, 0);
}

void hz_api_instance::success(const std::string &msg) {
    // Success format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = CTL_SUCCESS;
    send(sock, &word, sizeof(word), 0);

    uint64_t len = msg.length();
    send(sock, &len, sizeof(len), 0);

    send(sock, msg.c_str(), len, 0);
}

hz_api *hz_api::limit(uint64_t _max_instances) {
    max_instances = _max_instances;
    return this;
}

hz_api *hz_api::process(uint64_t _n_threads) {
    processor = rxnew(hz_processor, _n_threads);
    return this;
}

[[noreturn]] void hz_api::start(const char *addr, uint16_t port) {
    mutex = rnew(sem_t);
    sem_init(mutex, 0, max_instances);

    sockaddr_in server_addr{};
    sockaddr_in client_addr{};
    socklen_t sock_addr_size{};

    int server_sock = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(server_sock, (sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        throw ApiErrors::InitializationError(std::string("Failed to bind to socket addr: ") + std::string(addr));
    }

    if (listen(server_sock, max_instances) != 0) {
        throw ApiErrors::InitializationError("Failed to listen to socket");
    }

    while (true) {
        sock_addr_size = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr *) &client_addr, &sock_addr_size);

        char ip_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, INET_ADDRSTRLEN);

        LOG_F(INFO, "hzip.api: Accepted connection from %s:%d", ip_addr,
              (int) ntohs(client_addr.sin_port));

        hz_api_instance instance(client_sock, processor, passwd, mutex);

        sem_wait(mutex);
        sem_post(mutex);
    }
}

hz_api *hz_api::protect(const std::string& _passwd) {
    passwd = _passwd;
    return this;
}
