#ifndef HYBRIDZIP_VALIDATION_H
#define HYBRIDZIP_VALIDATION_H

#include <string>
#include <hzip/errors/utils.h>

inline void hz_validate_path(const std::string &path) {
    if (!path.starts_with('/')) {
        throw UtilErrors::ValidationError("Path must start with '/'");
    }
}

#endif
