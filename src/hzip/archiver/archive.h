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
    END = 0x4
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
    uint64_t obj_sof;
    uint8_t *data;
};

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
    sem_t rw_mutex;
    sem_t defrag_mutex;
    bool is_defrag_active;

    void blob_write(hza_block_info info, hzblob_t *blob);

    void atomic_defrag();

    void metadata_update();

    void scan();

    void scan_metadata_segment(const std::function<uint64_t(uint64_t)>& read);

public:
    hz_archive(std::string archive_path);

};

#endif
