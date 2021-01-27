#include "archive_provider.h"

rainman::ptr<HZ_Archive> hzapi::ArchiveProvider::provide(const std::string &path) {
    mutex.lock();
    rainman::ptr<HZ_Archive> archive;
    if (arch_map.contains(path)) {
        archive = arch_map[path];
    } else {
        archive = HZ_Archive(path);
        archive->load();

        arch_map[path] = archive;
    }

    mutex.unlock();

    return archive;
}

void hzapi::ArchiveProvider::close() {
    mutex.lock();

    for (auto &entry : arch_map) {
        entry.second->close();
    }

    mutex.unlock();
}
