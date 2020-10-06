#ifndef HYBRIDZIP_ARCHIVE_PROVIDER_H
#define HYBRIDZIP_ARCHIVE_PROVIDER_H

#include <unordered_map>
#include <string>
#include <semaphore.h>
#include <hzip/archive/archive.h>

namespace hzprovider {
    class archive {
    private:
        std::unordered_map<std::string, hz_archive*> arch_map;
        sem_t mutex;
        rainman::memmgr *mgr;
    public:
        void init(rainman::memmgr *_mgr);
        hz_archive *provide(const std::string &path);
        void close();
    };
}


#endif
