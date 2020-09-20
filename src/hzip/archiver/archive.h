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

#define HZ_ARCHIVE_VERSION 0x001000

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
    MSTATE_AUX = 0x2
};

struct hza_file {
    uint64_t *blob_ids;
    uint64_t blob_count;
    //todo: Add file information.
};

struct hza_fragment {
    uint64_t sof;
    uint64_t length;
};

template <typename T>
struct hza_entry {
    T data;
    uint64_t sof{};

    hza_entry(T data, uint64_t sof) {
        this->data = data;
        this->sof = sof;
    }
};

struct hza_metadata {
    std::string version;
    uint64_t eof;
    std::unordered_map<std::string, hza_entry<hza_file>> file_map;
    std::unordered_map<std::string, hza_entry<uint64_t>> mstate_aux_map;
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

    void hza_scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read,
                                   const std::function<void(uint64_t)> &seek);

    void hza_scan_blob_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void hza_scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void hza_scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    option_t<uint64_t> hza_alloc_fragment(uint64_t length);

    void hza_init();

    void hza_create_metadata_file_entry(const std::string& file_path, hza_file file);

    hza_file hza_read_metadata_file_entry(const std::string& file_path);

    hzblob_t *hza_read_blob(uint64_t id);

    uint64_t hza_write_blob(hzblob_t *blob);

    void hza_rm_blob(uint64_t id);

    hz_mstate *hza_read_mstate(uint64_t id);

    uint64_t hza_write_mstate(hz_mstate *mstate);

    void hza_rm_mstate(uint64_t id);

public:
    hz_archive(const std::string& archive_path);

    void create_file(const std::string& file_path, hzblob_t *blobs, uint64_t blob_count);

    hzblob_t *read_file(const std::string& file_path);

    void remove_file(const std::string& file_path);

    void install_mstate(const std::string &_path, hz_mstate *mstate);

    void uninstall_mstate(const std::string &_path);

    // Inject mstate by directly. This mstate cannot be reused by other blobs.
    void inject_mstate(hz_mstate *mstate, hzblob_t *blob);

    // Inject mstate by tag-reference. This mstate can be reused by other blobs.
    void inject_mstate(const std::string &_path, hzblob_t *blob);

    void close();
};

#endif
