#include "archive_provider.h"

hz_archive *hza_provider::provide(const std::string &path) {
    sem_wait(&mutex);
    hz_archive *archive = nullptr;
    if (arch_map.contains(path)) {
        archive = arch_map[path];
    } else {
        archive = mgr->r_malloc<hz_archive>(1);
        rinitptrfrom(mgr, archive);
        arch_map[path] = archive;
    }

    sem_post(&mutex);

    return archive;
}

void hza_provider::init(rainman::memmgr *_mgr) {
    sem_init(&mutex, 0, 1);
    mgr = _mgr;
}
