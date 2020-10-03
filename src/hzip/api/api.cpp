#include "api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <hzip/errors/api.h>
#include <hzip/utils/utils.h>

hz_api_instance::hz_api_instance(int _sock, hz_processor *_processor, const std::string &_passwd, sem_t *_mutex,
                                 char *_ip_addr, uint16_t _port) {
    processor = _processor;
    sock = _sock;
    passwd = _passwd;
    mutex = _mutex;
    port = _port;
    ip_addr = _ip_addr;
}

bool hz_api_instance::handshake() {
    uint64_t token = hz_rand64();
    uint64_t xtoken = hz_enc_token(passwd, token);

    if (t_send(&xtoken, sizeof(token))) {
        return true;
    }

    if (t_recv(&xtoken, sizeof(xtoken))) {
        return true;
    };

    if (token == xtoken) {
        LOG_F(INFO, "hzip.api: Handshake successful with %s:%d", ip_addr, port);
        success("Handshake successful.");
        return false;
    } else {
        LOG_F(INFO, "hzip.api: Handshake failed with %s:%d", ip_addr, port);
        error("Handshake failed.");
        return true;
    }
}

void hz_api_instance::error(const std::string &msg) {
    // Error format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = CTL_ERROR;
    HZ_SEND(&word, sizeof(word));

    uint64_t len = msg.length();

    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}

void hz_api_instance::success(const std::string &msg) {
    // Success format: <CTLWORD (1B)> <msg len (8B)> <msg (?B)>

    uint8_t word = CTL_SUCCESS;

    HZ_SEND(&word, sizeof(word));

    uint64_t len = msg.length();
    HZ_SEND(&len, sizeof(len));

    HZ_SEND(msg.c_str(), len);
}

void hz_api_instance::end() const {
    sem_post(mutex);
    close(sock);

    LOG_F(INFO, "hzip.api: Closed connection from %s:%d", ip_addr, port);

    rfree(ip_addr);
}

void hz_api_instance::start() {
    std::thread([this]() {
        sem_wait(mutex);
        if (handshake()) {
            end();
            return;
        }


        end();
    }).detach();
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

    LOG_F(INFO, "hzip.api: Started listening at port: %d", port);

    while (true) {
        sock_addr_size = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr *) &client_addr, &sock_addr_size);

        if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (uint8_t *) &time_out, sizeof(time_out)) < 0) {
            LOG_F(ERROR, "hzip.api: setsockopt() failed");
            continue;
        }

        char *ip_addr = rmalloc(char, INET_ADDRSTRLEN + 1);
        ip_addr[INET_ADDRSTRLEN] = '\0';

        inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, INET_ADDRSTRLEN);

        LOG_F(INFO, "hzip.api: Accepted connection from %s:%d", ip_addr,
              (int) ntohs(client_addr.sin_port));

        hz_api_instance instance(client_sock, processor, passwd, mutex, ip_addr, (int) ntohs(client_addr.sin_port));
        rinit(instance);

        instance.start();

        sem_wait(mutex);
        sem_post(mutex);
    }
}

hz_api *hz_api::protect(const std::string &_passwd) {
    passwd = _passwd;
    return this;
}

hz_api *hz_api::timeout(timeval _time_out) {
    time_out = _time_out;
    return this;
}
