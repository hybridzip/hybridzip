#include "archive.h"
#include <filesystem>
#include <fcntl.h>
#include <hzip/utils/utils.h>
#include <loguru/loguru.hpp>

hz_archive::hz_archive(const std::string& archive_path) {
    path = std::filesystem::absolute(archive_path);

    FILE *fp = fopen(path.c_str(), "rb+");
    stream = new bitio::stream(fp);

    char *sname = str_to_hex(path);

    // Lock archive.
    LOG_F(INFO, "hzip.archive: Requesting access to archive: %s", path.c_str());

    archive_mutex = sem_open(sname, O_CREAT, 0777, 1);
    sem_wait(archive_mutex);

    LOG_F(INFO, "hzip.archive: Access granted to archive: %s", path.c_str());

    mutex = new sem_t;
    sem_init(mutex, 0, 1);

    // todo: Create archive if non-existent

    scan();
}

void hz_archive::scan() {
    sem_wait(mutex);

    auto readfn = [this](uint64_t n) {
        this->metadata.eof += n;
        return this->stream->read(n);
    };

    auto seekfn = [this](uint64_t n) {
        this->metadata.eof += n;
        this->stream->seek(n);
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

    sem_post(mutex);
}

void hz_archive::scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read) {
    // Read block-info
    hza_block_info info{};

    info.marker = hza_marker::METADATA;
    info.sof = metadata.eof - 0x8;
    info.size = elias_gamma_inv(read).obj;

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
            // FILEINFO format: <path_length in elias-gamma> <file_path> <blob_count in elias-gamma>
            // <blob_id1 (64-bit)> <blob_id2 (64-bit)> ...

            uint64_t path_length = elias_gamma_inv(read).obj;
            char *_path = new char[path_length];

            for (uint64_t i = 0; i < path_length; i++) {
                _path[i] = read(0x8);
            }

            std::string file_path;

            file_path.assign(_path, path_length);
            free(_path);

            uint64_t blob_count = elias_gamma_inv(read).obj;
            auto *blob_ids = HZ_MALLOC(uint64_t, blob_count);

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
    auto size = elias_gamma_inv(read).obj;

    uint64_t blob_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.blob_map[blob_id] = sof;
}

void hz_archive::scan_journal_segment(const std::function<uint64_t(uint64_t)> &read) {
    // JOURNAL_ENTRY format: <block-size (elias-gamma)> | <jtask (1-bit)> <target_sof (64-bit)> <data (8-bit array)>
    // Read block-info
    auto size = elias_gamma_inv(read).obj;

    hza_journal_entry entry{};
    entry.task = (hza_jtask) read(0x1);
    entry.target_sof = read(0x40);
    entry.length = ((size - 0x41) >> 3);
    entry.data = HZ_MALLOC(uint8_t, entry.length);

    for (uint64_t i = 0; i < entry.length; i++) {
        entry.data[i] = read(0x8);
    }

    journal.entries.push_back(entry);
}

void
hz_archive::scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek) {
    hza_fragment fragment{};
    fragment.sof = metadata.eof - 0x8;
    fragment.length = elias_gamma_inv(read).obj;

    metadata.fragments.push_back(fragment);

    // Skip data block
    seek(fragment.length);
}

void hz_archive::scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read,
                                     const std::function<void(uint64_t)> &seek) {
    // MSTATE format: <block-length (elias-gamma)> <mstate-id (64-bit)> <mstate-data>

    auto sof = metadata.eof - 0x8;
    auto size = elias_gamma_inv(read).obj;

    uint64_t mstate_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.mstate_map[mstate_id] = sof;
}

option_t<uint64_t> hz_archive::alloc_fragment(uint64_t length) {
    uint64_t fragment_index = 0;
    uint64_t fit_diff = 0xffffffffffffffff;
    bin_t diff_bin{};
    bool found_fragment = false;

    // Best-fit for fragment utilization.
    for (uint64_t i = 0; i < metadata.fragments.size(); i++) {
        auto fragment = metadata.fragments[i];

        if (fragment.length > length) {
            uint64_t diff = fragment.length - length;
            if (diff < 0x48) {
                continue;
            }

            diff_bin = elias_gamma(diff - 0x8);

            if (diff < fit_diff) {
                fit_diff = diff;
                found_fragment = true;
                fragment_index = i;
            }
        }
    }

    if (found_fragment) {
        metadata.fragments[fragment_index].length = fit_diff - 0x48;
        uint64_t psof =  metadata.fragments[fragment_index].sof;
        metadata.fragments[fragment_index].sof += length;

        stream->seek_to(metadata.fragments[fragment_index].sof);
        stream->write(hza_marker::RESIDUAL, 0x8);
        stream->write(metadata.fragments[fragment_index].length, 0x40);

        return option_t<uint64_t>(psof);
    } else {
        return option_t<uint64_t>(0, false);
    }
}

void hz_archive::create_metadata_file_entry(const std::string& file_path, hza_metadata_file_entry entry) {
    // Evaluate length of entry to utilize fragmented-space.
    uint64_t length = 0;

    bin_t path_length = elias_gamma(file_path.length());
    length += path_length.n;
    length += (file_path.length() << 3);

    // Blob-count (n) (58-bit) + n 64-bit blob-ids
    length += 0x3A;
    length += (entry.file.blob_count << 0x6);

    option_t<uint64_t> o_frag = alloc_fragment(length);

    if (o_frag.is_valid) {
        uint64_t frag_sof = o_frag.get();

        // seek to allocated fragment.
        stream->seek_to(frag_sof);
    } else {
        // seek to end-of-file
        stream->seek_to(metadata.eof);
    }

    stream->write(hza_marker::BLOB, 0x8);
    stream->write(path_length.obj, path_length.n);

    // Write file_path
    for (int i = 0; i < file_path.length(); i++) {
        stream->write(file_path[i], 0x8);
    }

    // Write blob_count.
    stream->write(entry.file.blob_count, 0x3A);

    // Write 64-bit blob_ids
    for (int i = 0; i < entry.file.blob_count; i++) {
        stream->write(entry.file.blob_ids[i], 0x40);
    }

    sem_post(mutex);
}

void hz_archive::close() {
    stream->flush();
    stream->close();

    free(stream);
    free(mutex);

    sem_post(archive_mutex);
    free(archive_mutex);
}