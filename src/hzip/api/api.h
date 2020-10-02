#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <vector>
#include <thread>
#include <semaphore.h>
#include <rainman/rainman.h>
#include <hzip/processor/processor.h>
#include <netinet/in.h>

#define HZ_SEND(buf, n) if (t_send(buf, n)) return
#define HZ_RECV(buf, n) if (t_recv(buf, n)) return

enum CTL_WORD {
    CTL_SUCCESS = 0x0,
    CTL_ERROR = 0xff,
};

class hz_api_instance : public rainman::context {
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

    void error(const std::string &msg);

    void success(const std::string &msg);

    bool t_send(const void *buf, size_t n);

    bool t_recv(void *buf, size_t n);

    void end() const;
};

// An API based on socket-communication.
class hz_api : public rainman::context {
private:
    hz_processor *processor{};
    uint64_t max_instances = 1;
    sem_t *mutex{};
    std::string passwd = "hybridzip";

public:
    hz_api *limit(uint64_t _max_instances);

    hz_api *process(uint64_t _n_threads);

    hz_api *protect(const std::string &_passwd);

    [[noreturn]] void start(const char *addr, uint16_t port);
};

#endif
