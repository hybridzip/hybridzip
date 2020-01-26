#ifndef HYBRIDZIP_BOOST_UTILS_H
#define HYBRIDZIP_BOOST_UTILS_H

#include <boost/filesystem.hpp>
#include <hzip/other/platform.h>
#include <string>

namespace fs = boost::filesystem;

namespace hzboost {
    HZIP_FORCED_INLINE bool checkIfFileExists(std::string filename) {
        return fs::exists(fs::path{filename});
    }

    HZIP_FORCED_INLINE uint64_t getFileSize(std::string filename) {
        if (!fs::exists(fs::path{filename})) {

            return 0;
        }
        return fs::file_size(fs::path{filename});
    }

    HZIP_FORCED_INLINE void deleteFileIfExists(std::string filename) {
        if (fs::exists(fs::path{filename})) {
            fs::remove(fs::path{filename});
        }
    }
}


#endif //HYBRIDZIP_BOOST_UTILS_H
