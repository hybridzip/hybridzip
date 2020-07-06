#ifndef HYBRIDZIP_ERRORS_H
#define HYBRIDZIP_ERRORS_H

namespace HZERR {
    enum ERROR_TYPE {
        NO_ERROR = 0,
        BITIO_FILE_NOT_FOUND = 1,
        HZIP_CORE_ERROR = 2,
        HZIP_PATH_ERROR = 3
    };
}
#endif