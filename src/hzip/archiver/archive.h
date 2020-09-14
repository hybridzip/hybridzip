#ifndef HYBRIDZIP_ARCHIVE_H
#define HYBRIDZIP_ARCHIVE_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <semaphore.h>
#include <bitio/bitio.h>
#include <hzip/core/blob/hzblob.h>
#include <hzip/utils/common.h>
#include <hzip/memory/mem_interface.h>

enum hza_marker {
    EMPTY = 0x0,
    METADATA = 0x1,
    BLOB = 0x2,
    MSTATE = 0x3,
    END = 0xff
};

enum hza_metadata_entry_type {
    VERSION = 0x0,
    FILEINFO = 0x1,
};

struct hza_file {
    uint64_t *blob_ids;
    uint64_t blob_count;
    //todo: Add file information.
};

// Block-info
// marker: 8-bit
// sof: 64-bit
// size: elias-gamma :- represents the block size in bits.
struct hza_block_info {
    hza_marker marker;
    uint64_t sof;
    uint64_t size;
};

struct hza_fragment {
    uint64_t sof;
    uint64_t length;
};

struct hza_metadata_file_entry {
    hza_block_info info;
    hza_file file;
};

struct hza_metadata {
    std::string version;
    uint64_t eof;
    std::unordered_map<std::string, hza_metadata_file_entry> file_map;
    std::unordered_map<uint64_t, uint64_t> blob_map;
    std::unordered_map<uint64_t, uint64_t> mstate_map;
    std::vector<hza_fragment> fragments;
};

class hz_archive: public hz_mem_iface {
private:
    std::string path;
    hza_metadata metadata;
    bitio::stream *stream;
    sem_t *archive_mutex;
    sem_t *mutex;

    void hza_scan();

    void hza_scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read);

    void hza_scan_blob_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void hza_scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void hza_scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    option_t<uint64_t> hza_alloc_fragment(uint64_t length);

    void hza_create_metadata_file_entry(const std::string& file_path, hza_metadata_file_entry entry);

    void hza_write_blob(hzblob_t *blob);

    void hza_write_mstate(hz_mstate *mstate);

public:
    hz_archive(const std::string& archive_path);

    void close();
};

#endif
