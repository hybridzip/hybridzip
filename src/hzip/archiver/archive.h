#ifndef HYBRIDZIP_ARCHIVE_H
#define HYBRIDZIP_ARCHIVE_H

#include <unordered_map>
#include <bitio/bitio.h>
#include <hzip/core/blob/hzblob.h>

enum hza_marker {
    EMPTY = 0x0,
    METADATA = 0x1,
    JOURNAL = 0x2,
    BLOB = 0x3
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

struct hza_metadata_file {
    hza_file file;
    hza_block_info info;
};

struct hza_metadata {
    std::string version;
    std::unordered_map<std::string, hza_metadata_file> file_map;
};

class hz_archive {
private:
    bitio::stream *stream;

    void hza_blob_write(hza_block_info info, hzblob_t *blob);

public:

};

#endif
