#ifndef HYBRIDZIP_API_H
#define HYBRIDZIP_API_H

#include <vector>
#include <thread>
#include <semaphore>
#include <netinet/in.h>
#include <rainman/rainman.h>
#include <hzip_core/processor/processor.h>
#include "handlers/socket_class.h"
#include "providers/archive_provider.h"


namespace hzapi {
    class ApiInstance : public SocketInterface {
    private:
        rainman::ptr<HZ_Processor> _processor;
        std::string _password;
        rainman::ptr<std::counting_semaphore<>> _semaphore;
        rainman::ptr<hzapi::ArchiveProvider> _archive_provider;
    public:
        ApiInstance() = default;

        ApiInstance(int sock, const rainman::ptr<HZ_Processor> &processor, const std::string &password,
                    const rainman::ptr<std::counting_semaphore<>> &semaphore,
                    const std::string &ip_addr, uint16_t port,
                    const rainman::ptr<hzapi::ArchiveProvider> &archive_provider);

        void start();

        // The hzip_core api sends an encrypted random token to the client.
        // The client has to decrypt the token and send it back to the hzip_network api.
        void handshake();

        void end();
    };

// An API based on socket-communication.
    class Api {
    private:
        rainman::ptr<HZ_Processor> processor;
        uint64_t max_instances = 1;
        rainman::ptr<std::counting_semaphore<>> _semaphore;
        std::string passwd = "hybridzip";
        timeval time_out{};

        rainman::ptr<hzapi::ArchiveProvider> archive_provider;
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
