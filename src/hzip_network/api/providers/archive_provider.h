#ifndef HYBRIDZIP_ARCHIVE_PROVIDER_H
#define HYBRIDZIP_ARCHIVE_PROVIDER_H

#include <unordered_map>
#include <string>
#include <hzip_storage/archive/archive.h>

namespace hzapi {
    class ArchiveProvider {
    private:
        std::unordered_map<std::string, rainman::ptr<HZ_Archive>> arch_map;
        std::mutex mutex;
    public:
        rainman::ptr<HZ_Archive> provide(const std::string &path);

        void close();
    };
}


#endif
