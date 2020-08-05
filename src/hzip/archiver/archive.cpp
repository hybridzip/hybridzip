#include "archive.h"
#include <hzip/utils/utils.h>

hz_archive::hz_archive(std::string archive_path) {
    path = std::move(archive_path);
    sem_init(&rw_mutex, 0, 1);
    sem_init(&defrag_mutex, 0, 1);

    // todo: Create archive if non-existent

    is_defrag_active = false;

    scan();
}

void hz_archive::scan() {
    FILE *file = fopen(path.c_str(), "rb+");
    auto stream = new bitio::stream(file);

    auto readfn = [this, stream](uint64_t n) {
        this->metadata.eof += n;
        return stream->read(n);
    };

    auto seekfn = [this, stream](uint64_t n) {
        this->metadata.eof += n;
        stream->seek(n);
    };

    metadata.eof = 0;

    while (true) {
        auto marker = (hza_marker) readfn(0x8);

        if (marker == hza_marker::END) {
            break;
        }

        switch (marker) {
            case hza_marker::METADATA: {
                scan_metadata_segment(readfn);
                break;
            }
            case hza_marker::JOURNAL: {
                scan_journal_segment(readfn);
                break;
            }
            case hza_marker::BLOB: {
                scan_blob_segment(readfn, seekfn);
                break;
            }
            case hza_marker::MSTATE: {
                scan_mstate_segment(readfn, seekfn);
                break;
            }
            case hza_marker::EMPTY: {
                scan_fragment(readfn, seekfn);
                break;
            }
        }
    }
}

void hz_archive::scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read) {
    // Read block-info
    hza_block_info info{};

    info.marker = hza_marker::METADATA;
    info.sof = metadata.eof - 0x8;
    info.size = unaryinv_bin(read).obj;

    // Read metadata entry type
    auto entry_type = (hza_metadata_entry_type) read(0x1);

    switch (entry_type) {
        case hza_metadata_entry_type::VERSION: {
            // Version ranges from 0.0.0 to ff.ff.ff

            char version[3];

            version[0] = read(0x8);
            version[1] = read(0x8);
            version[2] = read(0x8);

            metadata.version = version;
            break;
        }
        case hza_metadata_entry_type::FILEINFO: {
            // FILEINFO format: <path_length in Elias Gamma Code> <file_path> <blob_count in Elias Gamma Code>
            // <blob_id1 (64-bit)> <blob_id2 (64-bit)> ...

            uint64_t path_length = unaryinv_bin(read).obj;
            char *_path = new char[path_length];

            for (uint64_t i = 0; i < path_length; i++) {
                _path[i] = read(0x8);
            }

            std::string file_path;

            file_path.assign(_path, path_length);
            free(_path);

            uint64_t blob_count = unaryinv_bin(read).obj;
            auto *blob_ids = new uint64_t[blob_count];

            for (uint64_t i = 0; i < blob_count; i++) {
                blob_ids[i] = read(0x40);
            }

            hza_file _hza_file{};
            _hza_file.blob_ids = blob_ids;
            _hza_file.blob_count = blob_count;

            hza_metadata_file_entry mf_entry{};
            mf_entry.file = _hza_file;
            mf_entry.info = info;

            metadata.file_map[file_path] = mf_entry;
            break;
        }
    }
}

void hz_archive::scan_blob_segment(const std::function<uint64_t(uint64_t)> &read,
                                   const std::function<void(uint64_t)> &seek) {
    // BLOB format: <block-length (elias-gamma)> <blob-id (64-bit)> <blob-data>

    auto sof = metadata.eof - 0x8;
    auto size = unaryinv_bin(read).obj;

    uint64_t blob_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.blob_map[blob_id] = sof;
}

void hz_archive::scan_journal_segment(const std::function<uint64_t(uint64_t)> &read) {
    // JOURNAL_ENTRY format: <block-size> | <jtask (1-bit)> <target_sof (64-bit)> <data (8-bit array)>
    // Read block-info
    auto size = unaryinv_bin(read).obj;

    hza_journal_entry entry{};
    entry.task = (hza_jtask) read(0x1);
    entry.target_sof = read(0x40);
    entry.length = ((size - 0x41) >> 3);
    entry.data = new uint8_t[entry.length];

    for (uint64_t i = 0; i < entry.length; i++) {
        entry.data[i] = read(0x8);
    }

    journal.entries.push_back(entry);
}

void hz_archive::scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek) {
    hza_fragment fragment{};
    fragment.sof = metadata.eof - 0x8;
    fragment.length = unaryinv_bin(read).obj;

    metadata.fragments.push_back(fragment);

    // Skip data block
    seek(fragment.length);
}

void hz_archive::scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read,
                                     const std::function<void(uint64_t)> &seek) {
    // MSTATE format: <block-length (elias-gamma)> <mstate-id (64-bit)> <mstate-data>

    auto sof = metadata.eof - 0x8;
    auto size = unaryinv_bin(read).obj;

    uint64_t mstate_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.mstate_map[mstate_id] = sof;
}
