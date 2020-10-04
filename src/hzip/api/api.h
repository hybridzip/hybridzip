#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <vector>
#include <thread>
#include <semaphore.h>
#include <netinet/in.h>
#include <rainman/rainman.h>
#include <hzip/processor/processor.h>
#include "hzip/api/handlers/socket_class.h"

class hz_api_instance : public rainman::context, public hz_socket_class {
private:
    hz_processor *processor{};
    int sock{};
    std::string passwd;
    sem_t *mutex{};
    char *ip_addr{};
    uint16_t port{};

public:
    hz_api_instance() = default;

    hz_api_instance(int _sock, hz_processor *_processor, const std::string &_passwd, sem_t *_mutex,
                    char *_ip_addr, uint16_t _port);

    void start();

    // The hzip api sends an encrypted random token to the client.
    // The client has to decrypt the token and send it back to the hzip api.
    bool handshake();

    void end() const;
};

// An API based on socket-communication.
class hz_api : public rainman::context {
private:
    hz_processor *processor{};
    uint64_t max_instances = 1;
    sem_t *mutex{};
    std::string passwd = "hybridzip";
    timeval time_out{};

public:
    hz_api *limit(uint64_t _max_instances);

    hz_api *process(uint64_t _n_threads);

    hz_api *timeout(timeval _time_out);

    hz_api *protect(const std::string &_passwd);

    [[noreturn]] void start(const char *addr, uint16_t port);
};

#endif
