#ifndef HYBRIDZIP_FSUTILS_H
#define HYBRIDZIP_FSUTILS_H

#include <string>
#include <filesystem>
#include <hzip_core/utils/platform.h>

namespace fsutils {
    HZ_INLINE void create_empty_file(const std::string &path) {
        fclose(fopen(path.c_str(), "wb"));
    }

    HZ_INLINE bool check_if_file_exists(const std::string& filename) {
        return std::filesystem::exists(filename) && !std::filesystem::is_directory(filename);
    }

    HZ_INLINE uint64_t get_file_size(const std::string& filename) {
        try {
            return std::filesystem::file_size(filename);
        } catch (...) {
            return 0;
        }
    }

    HZ_INLINE void delete_file_if_exists(const std::string &filename) {
        remove(filename.c_str());
    }
}

#endif
