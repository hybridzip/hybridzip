#include "api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <hzip/errors/api.h>
#include <hzip/utils/utils.h>
#include <hzip/api/handlers/stream.h>
#include <hzip/api/api_enums.h>

using namespace hzapi;

hz_api_instance::hz_api_instance(int _sock, hz_processor *_processor, const std::string &_passwd, sem_t *_mutex,
                                 char *_ip_addr, uint16_t _port, hzprovider::archive *_archive_provider) {
    processor = _processor;
    sock = _sock;
    passwd = _passwd;
    mutex = _mutex;
    port = _port;
    ip_addr = _ip_addr;
    archive_provider = _archive_provider;
}

bool hz_api_instance::handshake() {
    uint64_t token = hz_rand64();
    uint64_t xtoken = hz_enc_token(passwd, token);

    HZAPI_LOG(INFO, "Generated handshake token");

    HZ_SEND(&xtoken, sizeof(xtoken));
    HZ_RECV(&xtoken, sizeof(xtoken));

    if (token == xtoken) {
        HZAPI_LOG(INFO, "Handshake successful");
        success("Handshake successful");
        return false;
    } else {
        HZAPI_LOG(WARNING, "Handshake failed");
        error("Handshake failed");
        return true;
    }
}

void hz_api_instance::end() const {
    close(sock);
    HZAPI_LOG(INFO, "Closed connection");
    rfree(ip_addr);
    sem_post(mutex);
}

void hz_api_instance::start() {
    sem_wait(mutex);

    std::thread([this]() {
        HZAPI_LOG(INFO, "Instance created successfully");

        try {
            if (handshake()) {
                end();
                return;
            }

            auto streamer = rmod(hz_streamer, sock, ip_addr, port, processor, archive_provider);

            while (true) {
                uint8_t ctl_word;
                HZ_RECV(&ctl_word, sizeof(ctl_word));

                switch ((API_CTL) ctl_word) {
                    case API_CTL_STREAM: {
                        streamer.start();
                        break;
                    }
                    case API_CTL_QUERY: {

                        break;
                    }
                    case API_CTL_CLOSE: {
                        end();
                        return;
                    }
                    default: {
                        error("Invalid command");
                    }
                }
            }
        } catch (std::exception &e) {
            HZAPI_LOG(ERROR, e.what());
            error(e.what());
            end();
        }

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

    archive_provider = rnew(hzprovider::archive);
    archive_provider->init(rmemmgr);

    sockaddr_in server_addr{};
    sockaddr_in client_addr{};
    socklen_t sock_addr_size{};

    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr);

    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(server_sock, (sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        throw ApiErrors::InitializationError(std::string("Failed to bind to socket at: ") + std::string(addr));
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

        LOG_F(INFO, "hzip.api: [%s:%d] Accepted connection", ip_addr,
              (int) ntohs(client_addr.sin_port));

        hz_api_instance instance(client_sock, processor, passwd, mutex, ip_addr, (int) ntohs(client_addr.sin_port),
                                 archive_provider);
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

void hz_api::shutdown() {
    archive_provider->close();
    if (::shutdown(server_sock, SHUT_RDWR) < 0) {
        LOG_F(WARNING, "hzip.api: Socket shutdown failed with error=%s", strerror(errno));
    }
    close(server_sock);
}
