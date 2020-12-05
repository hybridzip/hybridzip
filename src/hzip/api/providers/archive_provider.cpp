#include "archive_provider.h"

HZ_Archive *hzapi::ArchiveProvider::provide(const std::string &path) {
    sem_wait(&mutex);
    HZ_Archive *archive;
    if (arch_map.contains(path)) {
        archive = arch_map[path];
    } else {
        archive = mgr->r_malloc<HZ_Archive>(1);
        *archive = HZ_Archive(path);
        rinitptrfrom(mgr, archive);
        archive->load();

        arch_map[path] = archive;
    }

    sem_post(&mutex);

    return archive;
}

void hzapi::ArchiveProvider::init(rainman::memmgr *_mgr) {
    sem_init(&mutex, 0, 1);
    mgr = _mgr;
}

void hzapi::ArchiveProvider::close() {
    sem_wait(&mutex);

    for (auto &entry : arch_map) {
        entry.second->close();
    }

    sem_post(&mutex);
}
