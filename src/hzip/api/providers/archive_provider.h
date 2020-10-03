#ifndef HYBRIDZIP_ARCHIVE_PROVIDER_H
#define HYBRIDZIP_ARCHIVE_PROVIDER_H

#include <unordered_map>
#include <string>
#include <semaphore.h>
#include <hzip/archive/archive.h>

class hza_provider {
private:
    static std::unordered_map<std::string, hz_archive*> arch_map;
    static sem_t mutex;
    static rainman::memmgr *mgr;
public:
    static void init(rainman::memmgr *_mgr);
    static hz_archive *provide(const std::string &path);
};

#endif
