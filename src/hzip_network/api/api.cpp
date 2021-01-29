#include "api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <hzip_core/utils/utils.h>
#include <hzip_network/errors/api.h>
#include "handlers/streamer.h"
#include "handlers/query.h"
#include "api_enums.h"

using namespace hzapi;

ApiInstance::ApiInstance(
        int sock,
        const rainman::ptr<HZ_Processor> &processor,
        const std::string &password,
        const rainman::ptr<std::counting_semaphore<>> &semaphore,
        const std::string &ip_addr,
        uint16_t port,
        const rainman::ptr<hzapi::ArchiveProvider> &archive_provider
) : SocketInterface(sock, ip_addr, port) {
    _processor = processor;
    _password = password;
    _semaphore = semaphore;
    _archive_provider = archive_provider;
}

void ApiInstance::handshake() {
    uint64_t token = hz_rand64();
    uint64_t xtoken = hz_enc_token(_password, token);

    HZAPI_LOG(INFO, "Generated handshake token");

    HZ_SEND(&xtoken, sizeof(xtoken));
    HZ_RECV(&xtoken, sizeof(xtoken));

    if (token == xtoken) {
        HZAPI_LOG(INFO, "Handshake successful");
        success("Handshake successful");
    } else {
        throw ApiErrors::InvalidOperationError("Handshake failed");
    }
}

void ApiInstance::end() {
    close(_sock);
    HZAPI_LOG(INFO, "Closed session");
    _semaphore->release();
}

void ApiInstance::start() {
    HZAPI_LOG(INFO, "Session created successfully");

    try {
        handshake();
    } catch (std::exception &e) {
        HZAPI_LOG(ERROR, e.what());
        end();
        return;
    }

    while (true) {
        try {
            uint8_t ctl_word;
            HZ_RECV(&ctl_word, sizeof(ctl_word));

            switch ((API_CTL) ctl_word) {
                case API_CTL_STREAM: {
                    auto streamer = Streamer(_sock, _ip_addr, _port, _processor, _archive_provider);
                    streamer.start();
                    break;
                }
                case API_CTL_QUERY: {
                    auto query = Query(_sock, _ip_addr, _port, _archive_provider);
                    query.start();
                    break;
                }
                case API_CTL_CLOSE: {
                    end();
                    return;
                }
                case API_CTL_HEALTH_CHECK: {
                    success("API is active and running");
                    break;
                }
                default: {
                    throw ApiErrors::InvalidOperationError("Invalid command");
                }
            }
        } catch (ApiErrors::ConnectionError &e) {
            HZAPI_LOG(ERROR, e.what());
            end();
            return;
        } catch (std::exception &e) {
            HZAPI_LOG(ERROR, e.what());
            error(e.what());
        }
    }

}

Api *Api::process(uint64_t _n_threads) {
    processor = rainman::ptr<HZ_Processor>(1, _n_threads);
    return this;
}

[[noreturn]] void Api::start(const char *addr, uint16_t port) {
    archive_provider = rainman::ptr<hzapi::ArchiveProvider>();

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

    LOG_F(INFO, "hzip_network.api: Started listening at port: %d", port);

    while (true) {
        sock_addr_size = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr *) &client_addr, &sock_addr_size);

        if (time_out.tv_sec > 0) {
            if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (uint8_t *) &time_out, sizeof(time_out)) < 0) {
                LOG_F(ERROR, "hzip_network.api: setsockopt() failed");
                continue;
            }

            if (setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (uint8_t *) &time_out, sizeof(time_out)) < 0) {
                LOG_F(ERROR, "hzip_network.api: setsockopt() failed");
                continue;
            }
        }

        char *ip_addr = new char[INET_ADDRSTRLEN + 1];
        ip_addr[INET_ADDRSTRLEN] = '\0';

        inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, INET_ADDRSTRLEN);

        LOG_F(INFO, "hzip_network.api: [%s:%d] Accepted connection", ip_addr,
              (int) ntohs(client_addr.sin_port));


        _semaphore->acquire();

        std::string ip = std::string(ip_addr, INET_ADDRSTRLEN + 1);

        std::thread([this, client_sock, ip, client_addr]() {
            ApiInstance instance(client_sock, processor, passwd, _semaphore, ip, (int) ntohs(client_addr.sin_port),
                                 archive_provider);

            instance.start();
        }).detach();


        _semaphore->acquire();
        _semaphore->release();
    }
}

Api *Api::protect(const std::string &_passwd) {
    passwd = _passwd;
    return this;
}

Api *Api::timeout(timeval _time_out) {
    time_out = _time_out;
    return this;
}

void Api::shutdown() {
    archive_provider->close();
    if (::shutdown(server_sock, SHUT_RDWR) < 0) {
        LOG_F(WARNING, "hzip_network.api: Socket shutdown failed with error (%s)", strerror(errno));
    }
    close(server_sock);
}

Api::Api(uint64_t max_instances) : _semaphore(max_instances) {

}
