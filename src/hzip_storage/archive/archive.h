#ifndef HYBRIDZIP_ARCHIVE_H
#define HYBRIDZIP_ARCHIVE_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <semaphore.h>
#include <bitio/bitio.h>
#include <rainman/rainman.h>
#include <hzip/core/blob/blob.h>
#include <hzip/utils/common.h>
#include <hzip/archive/archive_trie.h>


#define HZ_ARCHIVE_VERSION 0x000100

enum HZ_ArchiveMarker {
    EMPTY = 0x0,
    METADATA = 0x1,
    BLOB = 0x2,
    MSTATE = 0x3,
    END = 0xff
};

enum HZ_ArchiveMetadataEntryType {
    VERSION = 0x0,
    FILEINFO = 0x1,
    MSTATE_AUX = 0x2
};

struct HZ_ArchiveFile {
    rainman::ptr<uint64_t> blob_ids;
    uint64_t blob_count{};
    //todo: Add file information.
};

struct HZ_ArchiveFragment {
    uint64_t sof;
    uint64_t length;
};

template<typename T>
struct HZ_ArchiveEntry {
    T data;
    uint64_t sof{};

    HZ_ArchiveEntry() = default;

    HZ_ArchiveEntry(T data, uint64_t sof) {
        this->data = data;
        this->sof = sof;
    }
};

struct HZ_ArchiveMetadata {
    uint32_t version;
    uint64_t eof;
    HZ_ArchiveTrie<HZ_ArchiveEntry<HZ_ArchiveFile>> file_map;
    HZ_ArchiveTrie<HZ_ArchiveEntry<uint64_t>> mstate_aux_map;
    std::unordered_map<uint64_t, bool> mstate_inv_aux_map;
    std::unordered_map<uint64_t, uint64_t> blob_map;
    std::unordered_map<uint64_t, uint64_t> mstate_map;
    std::unordered_map<uint64_t, uint64_t> dep_counter;
    std::vector<HZ_ArchiveFragment> fragments;
};

class HZ_Archive {
private:
    std::string path;
    HZ_ArchiveMetadata metadata;
    bitio::stream *stream{};
    sem_t *archive_mutex{};
    sem_t *mutex{};

    void hza_scan();

    void hza_scan_metadata_segment(const std::function<uint64_t(uint64_t)> &read,
                                   const std::function<void(uint64_t)> &seek);

    void
    hza_scan_blob_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void
    hza_scan_mstate_segment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    void hza_scan_fragment(const std::function<uint64_t(uint64_t)> &read, const std::function<void(uint64_t)> &seek);

    option_t<uint64_t> hza_alloc_fragment(uint64_t length);

    void hza_init();

    void hza_create_metadata_file_entry(const std::string &file_path, HZ_ArchiveFile file);

    HZ_ArchiveFile hza_read_metadata_file_entry(const std::string &file_path);

    HZ_Blob *hza_read_blob(uint64_t id);

    uint64_t hza_write_blob(HZ_Blob *blob);

    void hza_rm_blob(uint64_t id);

    HZ_MState *hza_read_mstate(uint64_t id);

    uint64_t hza_write_mstate(HZ_MState *mstate);

    void hza_rm_mstate(uint64_t id);

    bool hza_check_mstate_deps(uint64_t id) const;

    void hza_increment_dep(uint64_t id);

    void hza_decrement_dep(uint64_t id);

    void hza_metadata_erase_file(const std::string &_file_path);

public:
    HZ_Archive() = default;

    HZ_Archive(const std::string &archive_path);

    void load();

    void create_file(const std::string &file_path, HZ_Blob *blobs, uint64_t blob_count);

    void create_file_entry(const std::string &file_path, HZ_ArchiveFile file);

    HZ_ArchiveFile read_file_entry(const std::string &file_path);

    bool check_file_exists(const std::string &file_path);

    uint64_t write_blob(HZ_Blob *blob);

    HZ_Blob *read_blob(uint64_t id);

    HZ_BlobSet read_file(const std::string &file_path);

    HZ_MState *read_mstate(std::string _path);

    void remove_file(const std::string &file_path);

    void install_mstate(const std::string &_path, HZ_MState *mstate);

    uint64_t install_mstate(HZ_MState *mstate);

    void uninstall_mstate(const std::string &_path);

    void uninstall_mstate(uint64_t id);

    // Inject mstate by directly. This mstate cannot be reused by other blobs.
    void inject_mstate(HZ_MState *mstate, HZ_Blob *blob);

    // Inject mstate by tag-reference. This mstate can be reused by other blobs.
    void inject_mstate(const std::string &_path, HZ_Blob *blob);

    bool check_mstate_exists(const std::string &_path);

    void close();

    std::vector<HZ_ArchiveTrieListElement> list_file_system(const std::string &prefix);

    std::vector<HZ_ArchiveTrieListElement> list_mstate_system(const std::string &prefix);
};

#endif
