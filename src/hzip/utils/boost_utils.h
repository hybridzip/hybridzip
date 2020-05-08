#ifndef HYBRIDZIP_BOOST_UTILS_H
#define HYBRIDZIP_BOOST_UTILS_H

#include <boost/filesystem.hpp>
#include <hzip/utils/platform.h>
#include <string>

namespace fs = boost::filesystem;

namespace boostutils {
    HZ_INLINE bool check_if_file_exists(std::string filename) {
        return fs::exists(fs::path{filename});
    }

    HZ_INLINE uint64_t get_file_size(std::string filename) {
        if (!fs::exists(fs::path{filename})) {

            return 0;
        }
        return fs::file_size(fs::path{filename});
    }

    HZ_INLINE void delete_file_if_exists(std::string filename) {
        if (fs::exists(fs::path{filename})) {
            fs::remove(fs::path{filename});
        }
    }
}

#endif
