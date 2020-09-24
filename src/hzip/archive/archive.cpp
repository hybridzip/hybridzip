#include "archive.h"
#include <filesystem>
#include <fcntl.h>
#include <hzip/utils/utils.h>
#include <hzip/utils/fsutils.h>
#include <loguru/loguru.hpp>
#include <hzip/errors/archive.h>

hz_archive::hz_archive(const std::string &archive_path) {
    path = std::filesystem::absolute(archive_path);

    bool init_flag = !fsutils::check_if_file_exists(path);

    if (init_flag) {
        fsutils::create_empty_file(path);
    }

    FILE *fp = fopen(path.c_str(), "rb+");
    stream = new bitio::stream(fp);

    auto sha2_path = sha512(path);

    const char *sname = sha2_path.c_str();

    // Lock archive.
    LOG_F(INFO, "hzip.archive: Requesting access to archive (%s)", path.c_str());

    archive_mutex = sem_open(sname, O_CREAT, 0777, 1);
    sem_wait(archive_mutex);

    LOG_F(INFO, "hzip.archive: Access granted to archive (%s)", path.c_str());

    mutex = new sem_t;
    sem_init(mutex, 0, 1);

    if (init_flag) {
        hza_init();
    }

    hza_scan();
}

void hz_archive::hza_scan() {
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
            metadata.eof -= 0x8;
            break;
        }

        switch (marker) {
            case hza_marker::METADATA: {
                hza_scan_metadata_segment(readfn, seekfn);
                break;
            }
            case hza_marker::BLOB: {
                hza_scan_blob_segment(readfn, seekfn);
                break;
            }
            case hza_marker::MSTATE: {
                hza_scan_mstate_segment(readfn, seekfn);
                break;
            }
            case hza_marker::EMPTY: {
                hza_scan_fragment(readfn, seekfn);
                break;
            }
            default:
                break;
        }
    }

    sem_post(mutex);
}

void hz_archive::hza_scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read,
                                           const std::function<void(uint64_t)> &seek) {

    uint64_t sof = metadata.eof;

    // block-length is not necessary, so skip it.
    seek(0x40);

    // Read metadata entry type
    auto entry_type = (hza_metadata_entry_type) read(0x8);

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
            uint64_t path_length = elias_gamma_inv(read).obj;
            char *_path = new char[path_length];

            for (uint64_t i = 0; i < path_length; i++) {
                _path[i] = read(0x8);
            }

            uint64_t blob_count = read(0x3A);
            auto blob_ids = HZ_MALLOC(uint64_t, blob_count);

            for (uint64_t i = 0; i < blob_count; i++) {
                blob_ids[i] = read(0x40);
            }

            auto file = hza_file{
                    .blob_ids=blob_ids,
                    .blob_count=blob_count
            };

            metadata.file_map[_path] = hza_entry(file, sof);

            break;
        }
        case hza_metadata_entry_type::MSTATE_AUX: {
            // mstate-auxillary map assigns a string to a mstate.
            // this is used when the same mstate is used by other blobs.
            // format: <path-length (elias-gamma)> <path (8-bit-array)> <mstate-id (64-bit)>

            uint64_t path_length = elias_gamma_inv(read).obj;
            char *_path = new char[path_length];

            for (uint64_t i = 0; i < path_length; i++) {
                _path[i] = read(0x8);
            }

            metadata.mstate_aux_map[_path] = hza_entry(stream->read(0x40), sof);
            break;
        }
    }
}

void hz_archive::hza_scan_blob_segment(const std::function<uint64_t(uint64_t)> &read,
                                       const std::function<void(uint64_t)> &seek) {
    // BLOB format: <hza_marker (8-bit)> <block-length (64bit)> <blob-id (64-bit)> ...

    auto sof = metadata.eof - 0x8;
    auto size = read(0x40);

    uint64_t blob_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.blob_map[blob_id] = sof;
}

void
hz_archive::hza_scan_fragment(const std::function<uint64_t(uint64_t)> &read,
                              const std::function<void(uint64_t)> &seek) {
    hza_fragment fragment{};
    fragment.sof = metadata.eof - 0x8;
    fragment.length = read(0x40);

    metadata.fragments.push_back(fragment);

    // Skip data block
    seek(fragment.length);
}

void hz_archive::hza_scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read,
                                         const std::function<void(uint64_t)> &seek) {
    // MSTATE format: <hza_marker (8-bit)> <block-length (64bit)> <mstate-id (64-bit)> ...

    auto sof = metadata.eof - 0x8;
    auto size = read(0x40);

    uint64_t mstate_id = read(0x40);

    // Skip the blob data
    seek(size - 0x40);

    metadata.mstate_map[mstate_id] = sof;
}

option_t<uint64_t> hz_archive::hza_alloc_fragment(uint64_t length) {
    uint64_t fragment_index = 0;
    uint64_t fit_diff = 0xffffffffffffffff;
    bool found_fragment = false;

    // Best-fit for fragment utilization.
    for (uint64_t i = 0; i < metadata.fragments.size(); i++) {
        auto fragment = metadata.fragments[i];

        if (fragment.length >= length) {
            uint64_t diff = fragment.length - length;
            if (diff < 0x48 && diff > 0) {
                continue;
            }

            if (diff < fit_diff) {
                fit_diff = diff;
                found_fragment = true;
                fragment_index = i;
            }
        }
    }

    if (found_fragment) {
        uint64_t psof = metadata.fragments[fragment_index].sof;

        if (fit_diff > 0) {
            metadata.fragments[fragment_index].length = fit_diff - 0x48;
            metadata.fragments[fragment_index].sof += 0x48 + length;

            stream->seek_to(metadata.fragments[fragment_index].sof);
            stream->write(hza_marker::EMPTY, 0x8);
            stream->write(metadata.fragments[fragment_index].length, 0x40);
        } else {
            // If perfect fit then erase fragment.
            metadata.fragments.erase(metadata.fragments.begin() + fragment_index);
        }

        return option_t<uint64_t>(psof);
    } else {
        return option_t<uint64_t>(0, false);
    }
}

void hz_archive::hza_init() {
    sem_wait(mutex);

    stream->seek_to(0);

    // Write archive version.
    stream->write(hza_marker::METADATA, 0x8);
    stream->write(0x20, 0x40);
    stream->write(hza_metadata_entry_type::VERSION, 0x8);
    stream->write(HZ_ARCHIVE_VERSION, 0x18);

    stream->write(hza_marker::END, 0x8);
    stream->flush();

    stream->seek_to(0);

    LOG_F(INFO, "hzip.archive: Initialized archive (%s)", path.c_str());

    sem_post(mutex);
}

void hz_archive::hza_create_metadata_file_entry(const std::string &file_path, hza_file file) {
    // Evaluate length of entry to utilize fragmented-space.

    if (metadata.file_map.contains(file_path)) {
        sem_post(mutex);
        throw ArchiveErrors::InvalidOperationException("file_entry_overwrite");
    }

    uint64_t length = 0;

    bin_t path_length = elias_gamma(file_path.length());
    length += path_length.n;
    length += (file_path.length() << 3);

    // FILEINFO-metadata-marker (8-bit) + Blob-count (n) (58-bit) + n 64-bit blob-ids
    length += 0x42 + (file.blob_count << 0x6);

    option_t<uint64_t> o_frag = hza_alloc_fragment(length);
    uint64_t sof;

    if (o_frag.is_valid) {
        uint64_t frag_sof = o_frag.get();
        sof = frag_sof;

        // seek to allocated fragment.
        stream->seek_to(frag_sof);
    } else {
        // seek to end-of-file
        sof = metadata.eof;

        stream->seek_to(metadata.eof);
        metadata.eof += 0x48 + length;
    }

    // Update metadata
    metadata.file_map[file_path] = hza_entry(file, sof);

    // Write block-info
    stream->write(hza_marker::METADATA, 0x8);
    stream->write(length, 0x40);

    stream->write(hza_metadata_entry_type::FILEINFO, 0x8);

    // Write file_path
    stream->write(path_length.obj, path_length.n);

    for (int i = 0; i < file_path.length(); i++) {
        stream->write(file_path[i], 0x8);
    }

    // Write blob_count.
    stream->write(file.blob_count, 0x3A);

    // Write 64-bit blob_ids
    for (int i = 0; i < file.blob_count; i++) {
        stream->write(file.blob_ids[i], 0x40);
    }

}

hza_file hz_archive::hza_read_metadata_file_entry(const std::string &file_path) {
    if (!metadata.file_map.contains(file_path)) {
        sem_post(mutex);
        throw ArchiveErrors::FileNotFoundException(file_path);
    }

    return metadata.file_map[file_path].data;
}

hzblob_t *hz_archive::hza_read_blob(uint64_t id) {
    auto blob = HZ_NEW(hzblob_t);
    HZ_MEM_INIT_PTR(blob);

    if (!metadata.blob_map.contains(id)) {
        sem_post(mutex);
        throw ArchiveErrors::BlobNotFoundException(id);
    }

    uint64_t sof = metadata.blob_map[id];

    stream->seek_to(sof);

    // Ignore block-marker, block-length and blob-id as we know the blob-format.
    stream->seek(0x88);


    blob->header = hz_blob_header();
    blob->header.length = stream->read(0x40);

    blob->header.raw = HZ_MALLOC(uint8_t, blob->header.length);

    for (uint64_t i = 0; i < blob->header.length; i++) {
        blob->header.raw[i] = stream->read(0x8);
    }

    blob->o_size = stream->read(0x40);
    blob->mstate_id = stream->read(0x40);

    blob->size = stream->read(0x40);
    blob->data = HZ_MALLOC(uint32_t, blob->size);

    for (uint64_t i = 0; i < blob->size; i++) {
        blob->data[i] = stream->read(0x20);
    }

    blob->mstate = hza_read_mstate(blob->mstate_id);

    return blob;
}

uint64_t hz_archive::hza_write_blob(hzblob_t *blob) {
    // blob writing format: <hzmarker (8bit)> <block length (64bit)> <blob-id (64-bit)>
    // <blob-header <size (64-bit)> <raw (8-bit-arr)>> <blob-o-size (64-bit)>
    // <mstate-id (64-bit)> <blob-data <size (64-bit)> <data (32-bit arr)>>

    if (!metadata.mstate_map.contains(blob->mstate_id)) {
        sem_post(mutex);
        throw ArchiveErrors::MstateNotFoundException(blob->mstate_id);
    }

    uint64_t length = 0x148 + (blob->header.length << 3) + (blob->size << 5);
    option_t<uint64_t> o_frag = hza_alloc_fragment(length);

    uint64_t sof;

    if (o_frag.is_valid) {
        sof = o_frag.get();
        stream->seek_to(sof);
    } else {
        sof = metadata.eof;
        stream->seek_to(sof);
        metadata.eof += 0x48 + length;
    }

    // Write block-info.
    stream->write(hza_marker::BLOB, 0x8);
    stream->write(length, 0x40);

    // Create a new unique blob-id and write it.
    uint64_t blob_id = rand64();
    while (metadata.blob_map.contains(blob_id)) {
        blob_id = rand64();
    }

    stream->write(blob_id, 0x40);

    // Update blob-map
    metadata.blob_map[blob_id] = sof;

    // Write blob metadata.
    stream->write(blob->header.length, 0x40);
    for (uint64_t i = 0; i < blob->header.length; i++) {
        stream->write(blob->header.raw[i], 0x8);
    }

    stream->write(blob->o_size, 0x40);
    stream->write(blob->mstate_id, 0x40);

    // add dependency
    hza_increment_dep(blob->mstate_id);

    // Write blob-data
    stream->write(blob->size, 0x40);
    for (int i = 0; i < blob->size; i++) {
        stream->write(blob->data[i], 0x20);
    }

    return blob_id;
}

void hz_archive::hza_rm_blob(uint64_t id) {
    if (metadata.blob_map.contains(id)) {
        uint64_t sof = metadata.blob_map[id];

        stream->seek_to(sof);
        stream->write(hza_marker::EMPTY, 0x8);

        metadata.fragments.push_back(hza_fragment{
                .sof=sof,
                .length=stream->read(0x40)
        });

        // remove dependency
        stream->seek(0x40);
        auto header_length = stream->read(0x40);
        stream->seek(header_length << 3);

        stream->seek(0x40);

        auto mstate_id = stream->read(0x40);
        hza_decrement_dep(mstate_id);

        // erase entry from blob map
        metadata.blob_map.erase(id);
    } else {
        LOG_F(WARNING, "hzip.archive: blob(0x%lx) was not found", id);
    }

}

hz_mstate *hz_archive::hza_read_mstate(uint64_t id) {
    if (!metadata.mstate_map.contains(id)) {
        sem_post(mutex);
        throw ArchiveErrors::MstateNotFoundException(id);
    }

    auto mstate = HZ_NEW(hz_mstate);
    uint64_t sof = metadata.mstate_map[id];

    stream->seek_to(sof);

    // Ignore block-marker, block-length and mstate-id as we know the mstate-format.
    stream->seek(0x88);

    mstate->alg = (hzcodec::algorithms::ALGORITHM) stream->read(0x8);

    auto length = stream->read(0x40);
    mstate->length = length;
    mstate->data = HZ_MALLOC(uint8_t, length);

    for (uint64_t i = 0; i < length; i++) {
        mstate->data[i] = stream->read(0x8);
    }

    return mstate;
}

uint64_t hz_archive::hza_write_mstate(hz_mstate *mstate) {
    // mstate writing format: <hzmarker (8bit)> <block length (64bit)>
    // <mstate-id (64-bit)> <algorithm (8-bit)> <data-length (64-bit)> <data (8-bit-array)>

    uint64_t length = 0x88 + (mstate->length << 0x3);
    option_t<uint64_t> o_frag = hza_alloc_fragment(length);

    uint64_t sof;

    if (o_frag.is_valid) {
        sof = o_frag.get();
        stream->seek_to(sof);
    } else {
        sof = metadata.eof;
        stream->seek_to(sof);
        metadata.eof += 0x48 + length;
    }

    // Write block-info.
    stream->write(hza_marker::MSTATE, 0x8);
    stream->write(length, 0x40);

    // Create new mstate-id
    uint64_t mstate_id = rand64();
    while (metadata.mstate_map.contains(mstate_id)) {
        mstate_id = rand64();
    }

    stream->write(mstate_id, 0x40);
    stream->write(mstate->alg, 0x8);

    // Update mstate-map
    metadata.mstate_map[mstate_id] = sof;

    // Write mstate-data
    stream->write(mstate->length, 0x40);

    for (int i = 0; i < mstate->length; i++) {
        stream->write(mstate->data[i], 0x8);
    }

    return mstate_id;
}

void hz_archive::install_mstate(const std::string &_path, hz_mstate *mstate) {
    sem_wait(mutex);
    if (metadata.mstate_aux_map.contains(_path)) {
        sem_post(mutex);
        throw ArchiveErrors::InvalidOperationException("mstate_aux_entry_overwrite");
    }

    uint64_t id = hza_write_mstate(mstate);

    // Create auxillary mstate-entry.
    uint64_t length = 0;

    bin_t path_length = elias_gamma(_path.length());
    length += path_length.n;
    length += (_path.length() << 3) + 0x48;

    option_t<uint64_t> o_frag = hza_alloc_fragment(length);

    uint64_t sof;
    if (o_frag.is_valid) {
        sof = o_frag.get();

        // seek to allocated fragment.
        stream->seek_to(sof);
    } else {
        sof = metadata.eof;

        // seek to end-of-file
        stream->seek_to(metadata.eof);
        metadata.eof += 0x48 + length;
    }

    // Update metadata
    metadata.mstate_aux_map[_path] = hza_entry(id, sof);

    // Write block-info
    stream->write(hza_marker::METADATA, 0x8);
    stream->write(length, 0x40);

    stream->write(hza_metadata_entry_type::MSTATE_AUX, 0x8);

    // Write file_path
    stream->write(path_length.obj, path_length.n);

    for (int i = 0; i < _path.length(); i++) {
        stream->write(_path[i], 0x8);
    }

    // Write mstate-id.
    stream->write(id, 0x40);

    sem_post(mutex);
}

void hz_archive::hza_rm_mstate(uint64_t id) {
    if (metadata.mstate_map.contains(id)) {
        if (hza_check_mstate_deps(id)) {
            sem_post(mutex);
            throw ArchiveErrors::InvalidOperationException("mstate_dependency_detected");
        }

        uint64_t sof = metadata.mstate_map[id];

        stream->seek_to(sof);
        stream->write(hza_marker::EMPTY, 0x8);

        metadata.fragments.push_back(hza_fragment{
                .sof=sof,
                .length=stream->read(0x40)
        });

        metadata.mstate_map.erase(id);
    } else {
        LOG_F(WARNING, "hzip.archive: mstate(0x%lx) was not found", id);
    }

}

bool hz_archive::hza_check_mstate_deps(uint64_t id) const {
    return metadata.dep_counter.contains(id);
}

void hz_archive::hza_increment_dep(uint64_t id) {
    if (!metadata.dep_counter.contains(id)) {
        metadata.dep_counter[id] = 1;
    } else {
        metadata.dep_counter[id]++;
    }
}

void hz_archive::hza_decrement_dep(uint64_t id) {
    if (metadata.dep_counter.contains(id)) {
        if (metadata.dep_counter[id] > 1) {
            metadata.dep_counter[id]--;
        } else {
            metadata.dep_counter.erase(id);
        }
    }
}

void hz_archive::create_file(const std::string &file_path, hzblob_t *blobs, uint64_t blob_count) {
    sem_wait(mutex);
    if (metadata.file_map.contains(file_path)) {
        sem_post(mutex);
        throw ArchiveErrors::InvalidOperationException("file_already_exists");
    }

    hza_file file{};
    file.blob_count = blob_count;
    file.blob_ids = HZ_MALLOC(uint64_t, blob_count);

    for (uint64_t i = 0; i < blob_count; i++) {
        file.blob_ids[i] = hza_write_blob(&blobs[i]);
    }

    hza_create_metadata_file_entry(file_path, file);

    sem_post(mutex);
}

void hz_archive::remove_file(const std::string &file_path) {
    sem_wait(mutex);
    if (!metadata.file_map.contains(file_path)) {
        sem_post(mutex);
        throw ArchiveErrors::FileNotFoundException(file_path);
    }

    hza_entry entry = metadata.file_map[file_path];

    uint64_t sof = entry.sof;
    hza_file file = entry.data;

    metadata.file_map.erase(file_path);

    stream->seek_to(sof);
    stream->write(hza_marker::EMPTY, 0x8);

    metadata.fragments.push_back(hza_fragment{
            .sof=sof,
            .length=stream->read(0x40)
    });

    for (uint64_t i = 0; i < file.blob_count; i++) {
        hza_rm_blob(file.blob_ids[i]);
    }

    sem_post(mutex);
}

hzblob_set hz_archive::read_file(const std::string &file_path) {
    sem_wait(mutex);
    if (!metadata.file_map.contains(file_path)) {
        sem_post(mutex);
        throw ArchiveErrors::FileNotFoundException(file_path);
    }

    hza_file file = hza_read_metadata_file_entry(file_path);

    auto *blobs = HZ_MALLOC(hzblob_t, file.blob_count);

    for (uint64_t i = 0; i < file.blob_count; i++) {
        auto blob = hza_read_blob(file.blob_ids[i]);
        blobs[i] = *blob;
        HZ_FREE(blob);
    }

    sem_post(mutex);

    return hzblob_set{.blobs=blobs, .blob_count=file.blob_count};
}

void hz_archive::uninstall_mstate(const std::string &_path) {
    sem_wait(mutex);
    if (!metadata.mstate_aux_map.contains(_path)) {
        sem_post(mutex);
        throw ArchiveErrors::MstateNotFoundException(0);
    }

    hza_entry entry = metadata.mstate_aux_map[_path];

    uint64_t sof = entry.sof;
    uint64_t id = entry.data;

    hza_rm_mstate(id);

    stream->seek_to(sof);
    stream->write(hza_marker::EMPTY, 0x8);

    metadata.fragments.push_back(hza_fragment{
            .sof=sof,
            .length=stream->read(0x40)
    });

    metadata.mstate_aux_map.erase(_path);

    sem_post(mutex);
}

void hz_archive::inject_mstate(hz_mstate *mstate, hzblob_t *blob) {
    sem_wait(mutex);

    blob->mstate_id = hza_write_mstate(mstate);
    blob->mstate = mstate;

    sem_post(mutex);
}

void hz_archive::inject_mstate(const std::string &_path, hzblob_t *blob) {
    sem_wait(mutex);

    if (!metadata.mstate_aux_map.contains(_path)) {
        sem_post(mutex);
        throw ArchiveErrors::MstateNotFoundException(0);
    }

    blob->mstate_id = metadata.mstate_aux_map[_path].data;

    blob->mstate = hza_read_mstate(blob->mstate_id);

    sem_post(mutex);
}

uint64_t hz_archive::install_mstate(hz_mstate *mstate) {
    sem_wait(mutex);
    uint64_t id = hza_write_mstate(mstate);
    sem_post(mutex);
    return id;
}

void hz_archive::uninstall_mstate(uint64_t id) {
    sem_wait(mutex);
    hza_rm_mstate(id);
    sem_post(mutex);
}

void hz_archive::close() {
    sem_wait(mutex);

    stream->seek_to(metadata.eof);
    stream->write(hza_marker::END, 0x8);
    stream->flush();
    stream->close();

    sem_post(mutex);

    delete stream;
    delete mutex;

    sem_post(archive_mutex);
}
