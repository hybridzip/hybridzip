#ifndef HYBRIDZIP_VALIDATION_H
#define HYBRIDZIP_VALIDATION_H

#include <string>
#include <hzip_core/errors/utils.h>

inline void hz_validate_path(const std::string &path) {
    if (!path.starts_with('/')) {
        throw UtilErrors::ValidationError("Path must start with '/'");
    }

    for (char c : path) {
        if (c == '\0') {
            throw UtilErrors::ValidationError("Path must not contain null values");
        } else if (c == '\\') {
            throw UtilErrors::ValidationError("Path must not contain back slash");
        } else if (c == ' ') {
            throw UtilErrors::ValidationError("Path must not contain spaces");
        }
    }

    for (int i = 0; i < path.length() - 1; i++) {
        if (path[i] == '/' && path[i + 1] == '/') {
            throw UtilErrors::ValidationError("Path must not contain repeated forward slashes");
        }
    }
}

#endif
