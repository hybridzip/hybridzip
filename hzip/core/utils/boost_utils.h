#ifndef HYBRIDZIP_BOOST_UTILS_H
#define HYBRIDZIP_BOOST_UTILS_H

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

uint64_t getFileSize(char* filename) {
    if (!fs::exists(fs::path{filename})) {

        return 0;
    }
    return fs::file_size(fs::path{filename});
}

void deleteFileIfExists(char* filename) {
    if(fs::exists(fs::path{filename})) {
        fs::remove(fs::path{filename});
    }
}

#endif //HYBRIDZIP_BOOST_UTILS_H
