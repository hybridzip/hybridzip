#include "archive_provider.h"

hz_archive *hzprovider::archive::provide(const std::string &path) {
    sem_wait(&mutex);
    hz_archive *archive;
    if (arch_map.contains(path)) {
        archive = arch_map[path];
    } else {
        archive = mgr->r_malloc<hz_archive>(1);
        *archive = hz_archive(path);
        rinitptrfrom(mgr, archive);
        archive->load();

        arch_map[path] = archive;
    }

    sem_post(&mutex);

    return archive;
}

void hzprovider::archive::init(rainman::memmgr *_mgr) {
    sem_init(&mutex, 0, 1);
    mgr = _mgr;
}

void hzprovider::archive::close() {
    sem_wait(&mutex);

    for (auto &entry : arch_map) {
        entry.second->close();
    }

    sem_post(&mutex);
}
