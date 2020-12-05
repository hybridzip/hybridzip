#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <vector>
#include <thread>
#include <semaphore.h>
#include <netinet/in.h>
#include <rainman/rainman.h>
#include <hzip/processor/processor.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/providers/archive_provider.h>

namespace hzapi {
    class ApiInstance : public rainman::context, public SocketInterface {
    private:
        HZ_Processor *processor{};
        std::string passwd;
        sem_t *mutex{};

        hzapi::ArchiveProvider *archive_provider{};
    public:
        ApiInstance() = default;

        ApiInstance(int _sock, HZ_Processor *_processor, const std::string &_passwd, sem_t *_mutex,
                    char *_ip_addr, uint16_t _port, hzapi::ArchiveProvider *_archive_provider);

        void start();

        // The hzip api sends an encrypted random token to the client.
        // The client has to decrypt the token and send it back to the hzip api.
        void handshake();

        void end();
    };

// An API based on socket-communication.
    class Api : public rainman::context {
    private:
        HZ_Processor *processor{};
        uint64_t max_instances = 1;
        sem_t *mutex{};
        std::string passwd = "hybridzip";
        timeval time_out{};

        hzapi::ArchiveProvider *archive_provider{};
        int server_sock{};

    public:
        Api *limit(uint64_t _max_instances);

        Api *process(uint64_t _n_threads);

        Api *timeout(timeval _time_out);

        Api *protect(const std::string &_passwd);

        void shutdown();

        [[noreturn]] void start(const char *addr, uint16_t port);
    };
}

#endif
