#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>
#include <hzip/archive/archive.h>
#include <hzip/core/blob/hzblob.h>

struct hz_job {
    char *tag;
    char *dest;
    hzblob_t *blob;
    hz_archive *archive;
    hzcodec::algorithms::ALGORITHM algorithm;
};

#endif
