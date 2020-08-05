#ifndef HYBRIDZIP_ARCHIVE_H
#define HYBRIDZIP_ARCHIVE_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <semaphore.h>
#include <hzip/core/blob/hzblob.h>
#include <bitio/bitio.h>

#define HZ_ARCHIVE_VERSION {0, 1, 0}

enum hza_marker {
    EMPTY = 0x0,
    METADATA = 0x1,
    JOURNAL = 0x2,
    BLOB = 0x3,
    MSTATE = 0x4,
    END = 0xff
};

enum hza_metadata_entry_type {
    VERSION = 0x0,
    FILEINFO = 0x1,
};

enum hza_jtask {
    WRITE = 0x0,
    DEFRAG = 0x1
};


struct hza_file {
    uint64_t *blob_ids;
    uint64_t blob_count;
    //todo: Add file information.
};

struct hza_journal_entry {
    hza_jtask task;
    uint64_t target_sof;
    uint64_t length;
    uint8_t *data;
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

struct hza_journal {
    std::vector<hza_journal_entry> entries;
};

class hz_archive {
private:
    std::string path;
    hza_metadata metadata;
    hza_journal journal;

    // maps connection id to a bitio::stream
    std::unordered_map<uint64_t, bitio::stream*> stream_map;

    sem_t rw_mutex;
    sem_t defrag_mutex;
    bool is_defrag_active;

    void scan();

    void scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read);

    void scan_blob_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void scan_journal_segment(const std::function<uint64_t(uint64_t)> &read);

    void scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void delete_at_sof(uint64_t sof);
public:
    struct hza_connection {
        uint64_t id;
        hz_archive *archive;
        // todo: Implement archive permissions

        void close();
    };

    hz_archive(std::string archive_path);

    hza_connection create_conn();
};

#endif
