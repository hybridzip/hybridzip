#ifndef HYBRIDZIP_FSUTILS_H
#define HYBRIDZIP_FSUTILS_H

#include <hzip/utils/platform.h>
#include <string>
#include <filesystem>

namespace fsutils {
    HZ_INLINE bool check_if_file_exists(std::string filename) {
        return std::filesystem::exists(filename);
    }

    HZ_INLINE uint64_t get_file_size(std::string filename) {
        try {
            return std::filesystem::file_size(filename);
        } catch (...) {
            return 0;
        }
    }

    HZ_INLINE void delete_file_if_exists(const std::string& filename) {
        remove(filename.c_str());
    }
}

#endif
