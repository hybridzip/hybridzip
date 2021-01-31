#include "query.h"
#include <hzip_core/utils/validation.h>
#include <hzip_network/api/api_enums.h>
#include <hzip_network/errors/api.h>

using namespace hzapi;

Query::Query(
        int sock,
        const std::string &ip_addr,
        uint16_t port,
        const rainman::ptr<hzapi::ArchiveProvider> &archive_provider
) : SocketInterface(sock, ip_addr, port) {
    _archive_provider = archive_provider;
}

void Query::start() {
    rainman::option<rainman::ptr<HZ_Archive>> archive;
    rainman::ptr<char> archive_path;
    uint16_t archive_path_len = 0;
    rainman::option<rainman::ptr<char>> target;
    uint16_t target_len = 0;
    bool piggy_back = false;

    while (true) {
        uint8_t word;
        HZ_RECV(&word, sizeof(word));

        switch ((QUERY_CTL) word) {
            case QUERY_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rainman::ptr<char>(archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path.pointer(), archive_path_len);

                archive = _archive_provider->provide(archive_path.pointer());
                break;
            }
            case QUERY_CTL_CHECK_IF_FILE_EXISTS: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggyback was disabled");
                }

                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (target.is_none()) {
                    throw ApiErrors::InvalidOperationError("Target not provided");
                }

                bool found = archive.inner()->check_file_exists(target.inner().pointer());
                HZ_SEND(&found, sizeof(found));

                return;
            }
            case QUERY_CTL_LIST_FILE_SYSTEM: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggyback was disabled");
                }

                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (target.is_none()) {
                    throw ApiErrors::InvalidOperationError("Target not provided");
                }

                auto list = archive.inner()->list_file_system(target.inner().pointer());

                uint64_t count = list.size();
                HZ_SEND(&count, sizeof(count));

                for (const auto &elem : list) {
                    uint16_t len = elem.entry.length();
                    HZ_SEND(&len, sizeof(len));
                    HZ_SEND(elem.entry.c_str(), len);
                    HZ_SEND(&elem.is_leaf, sizeof(elem.is_leaf));
                }

                return;
            }
            case QUERY_CTL_LIST_MSTATE_SYSTEM: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggyback was disabled");
                }

                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (target.is_none()) {
                    throw ApiErrors::InvalidOperationError("Target not provided");
                }

                auto list = archive.inner()->list_mstate_system(target.inner().pointer());

                uint64_t count = list.size();
                HZ_SEND(&count, sizeof(count));

                for (const auto &elem : list) {
                    uint16_t len = elem.entry.length();
                    HZ_SEND(&len, sizeof(len));
                    HZ_SEND(elem.entry.c_str(), len);
                    HZ_SEND(&elem.is_leaf, sizeof(elem.is_leaf));
                }

                return;
            }
            case QUERY_CTL_PIGGYBACK: {
                piggy_back = true;
                break;
            }
            case QUERY_CTL_DELETE_FILE: {
                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (target.is_none()) {
                    throw ApiErrors::InvalidOperationError("Target not provided");
                }

                archive.inner()->remove_file(target.inner().pointer());
                return;
            }
            case QUERY_CTL_DELETE_MSTATE: {
                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (target.is_none()) {
                    throw ApiErrors::InvalidOperationError("Target not provided");
                }

                archive.inner()->uninstall_mstate(target.inner().pointer());
                return;
            }
            case QUERY_CTL_GET_MEM_USAGE: {
                uint64_t alloc_size = rainman::Allocator().alloc_size();
                HZ_SEND(&alloc_size, sizeof(alloc_size));

                return;
            }
            case QUERY_CTL_TARGET: {
                HZ_RECV(&target_len, sizeof(target_len));

                target = rainman::ptr<char>(target_len + 1);
                target.inner()[target_len] = 0;

                HZ_RECV(target.inner().pointer(), target_len);
                hz_validate_path(target.inner().pointer());

                break;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}
