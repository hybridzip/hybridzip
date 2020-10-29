#include "query.h"
#include <hzip/api/api_enums.h>
#include <hzip/errors/api.h>
#include <hzip/utils/validation.h>

using namespace hzapi;

hz_query::hz_query(int _sock, char *_ip_addr, uint16_t _port, hzprovider::archive *_archive_provider) {
    sock = _sock;
    ip_addr = _ip_addr;
    port = _port;
    archive_provider = _archive_provider;
}

void hz_query::start() {
    hz_archive *archive = nullptr;
    char *archive_path = nullptr;
    uint16_t archive_path_len = 0;
    char *dest = nullptr;
    uint16_t dest_len = 0;
    char *mstate_addr = nullptr;
    uint16_t mstate_addr_len = 0;
    bool piggy_back = false;

    while (true) {
        uint8_t word;
        HZ_RECV(&word, sizeof(word));

        switch ((QUERY_CTL) word) {
            case QUERY_CTL_ARCHIVE: {
                if (archive_path != nullptr) {
                    rfree(archive_path);
                }

                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path, archive_path_len);

                archive = archive_provider->provide(archive_path);
                break;
            }
            case QUERY_CTL_CHECK_IF_FILE_EXISTS: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggyback was disabled");
                }

                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                HZ_RECV(&dest_len, sizeof(dest_len));

                dest = rmalloc(char, dest_len + 1);
                dest[dest_len] = 0;

                HZ_RECV(dest, dest_len);
                hz_validate_path(dest);

                bool found = archive->check_file_exists(dest);
                HZ_SEND(&found, sizeof(found));

                return;
            }
            case QUERY_CTL_GET_ALL_FILES: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggyback was disabled");
                }

                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                auto files = archive->list_files();

                uint64_t count = files.size();
                HZ_SEND(&count, sizeof(count));

                for (auto file : files) {
                    uint16_t len = file.length();
                    HZ_SEND(&len, sizeof(len));
                    HZ_SEND(file.c_str(), len);
                }

                return;
            }
            case QUERY_CTL_PIGGYBACK: {
                piggy_back = true;
                break;
            }
            case QUERY_CTL_DELETE_FILE: {
                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                HZ_RECV(&dest_len, sizeof(dest_len));

                dest = rmalloc(char, dest_len + 1);
                dest[dest_len] = 0;

                HZ_RECV(dest, dest_len);
                hz_validate_path(dest);

                archive->remove_file(dest);
                return;
            }
            case QUERY_CTL_DELETE_MSTATE: {
                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len + 1);
                mstate_addr[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr, mstate_addr_len);
                hz_validate_path(mstate_addr);

                archive->uninstall_mstate(mstate_addr);
                return;
            }
            case QUERY_CTL_GET_MEM_USAGE: {
                uint64_t alloc_size = rparentmgr->get_alloc_size();
                HZ_SEND(&alloc_size, sizeof(alloc_size));

                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}
