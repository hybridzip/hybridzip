#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>

struct hz_job {
    uint8_t *data;
    char *tag;
    char *data_class;
    char *dest;
    char *archive_path;
};

#endif
