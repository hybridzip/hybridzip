#include "api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hzip/errors/api.h>

hz_api_instance::hz_api_instance(int _sock, hz_processor *_processor, sem_t *mutex) {
    processor = _processor;
    sock = _sock;
    thread = std::thread([this, mutex]() {
        sem_wait(mutex);


        sem_post(mutex);
    });
}

void hz_api_instance::handshake() {

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
    sockaddr_storage server_storage{};
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
        sock_addr_size = sizeof(server_storage);
        int client_sock = accept(server_sock, (sockaddr *) &server_storage, &sock_addr_size);

        hz_api_instance instance(client_sock, processor, mutex);

        sem_wait(mutex);
        sem_post(mutex);
    }
}
