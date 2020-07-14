#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>
#include <hzip/archiver/archiver.h>

struct hz_job {
    uint8_t *data;
    char *tag;
    char *dest;
    hz_archive *archive;
};

#endif
