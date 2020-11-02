#ifndef HYBRIDZIP_ARCHIVE_H
#define HYBRIDZIP_ARCHIVE_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <semaphore.h>
#include <bitio/bitio.h>
#include <rainman/rainman.h>
#include <hzip/core/blob/hzblob.h>
#include <hzip/utils/common.h>


#define HZ_ARCHIVE_VERSION 0x000100

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

struct hza_file : public rainman::context {
    uint64_t *blob_ids;
    uint64_t blob_count;
    //todo: Add file information.

    void destroy() {
        rfree(blob_ids);
    }
};

struct hza_fragment {
    uint64_t sof;
    uint64_t length;
};

template <typename T>
struct hza_entry {
    T data;
    uint64_t sof{};

    hza_entry() = default;

    hza_entry(T data, uint64_t sof) {
        this->data = data;
        this->sof = sof;
    }
};

struct hza_metadata {
    uint32_t version;
    uint64_t eof;
    std::unordered_map<std::string, hza_entry<hza_file>> file_map;
    std::unordered_map<std::string, hza_entry<uint64_t>> mstate_aux_map;
    std::unordered_map<uint64_t, bool> mstate_inv_aux_map;
    std::unordered_map<uint64_t, uint64_t> blob_map;
    std::unordered_map<uint64_t, uint64_t> mstate_map;
    std::unordered_map<uint64_t, uint64_t> dep_counter;
    std::vector<hza_fragment> fragments;
};

class hz_archive: public rainman::context {
private:
    std::string path;
    hza_metadata metadata;
    bitio::stream *stream{};
    sem_t *archive_mutex{};
    sem_t *mutex{};

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

    bool hza_check_mstate_deps(uint64_t id) const;

    void hza_increment_dep(uint64_t id);

    void hza_decrement_dep(uint64_t id);

    void hza_metadata_erase_file(const std::string &_file_path);

public:
    hz_archive() = default;

    hz_archive(const std::string& archive_path);

    void load();

    std::vector<std::string> list_files();

    std::vector<std::string> list_mstates();

    void create_file(const std::string& file_path, hzblob_t *blobs, uint64_t blob_count);

    void create_file_entry(const std::string& file_path, hza_file file);

    hza_file read_file_entry(const std::string& file_path);

    bool check_file_exists(const std::string& file_path);

    uint64_t write_blob(hzblob_t *blob);

    hzblob_t *read_blob(uint64_t id);

    hzblob_set read_file(const std::string& file_path);

    hz_mstate *read_mstate(std::string _path);

    void remove_file(const std::string& file_path);

    void install_mstate(const std::string &_path, hz_mstate *mstate);

    uint64_t install_mstate(hz_mstate *mstate);

    void uninstall_mstate(const std::string &_path);

    void uninstall_mstate(uint64_t id);

    // Inject mstate by directly. This mstate cannot be reused by other blobs.
    void inject_mstate(hz_mstate *mstate, hzblob_t *blob);

    // Inject mstate by tag-reference. This mstate can be reused by other blobs.
    void inject_mstate(const std::string &_path, hzblob_t *blob);

    bool check_mstate_exists(const std::string &_path);

    void close();
};

#endif
