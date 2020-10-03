#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>
#include <functional>
#include <hzip/archive/archive.h>
#include <hzip/core/blob/hzblob.h>

struct hz_codec_job {
    enum JOBTYPE {
        ENCODE = 0x0,
        DECODE = 0x1,
    };

    hzblob_t *blob;
    hz_archive *archive;
    hzcodec::algorithms::ALGORITHM algorithm;
    JOBTYPE job_type;
    bool reuse_mstate;
    std::string mstate_addr;
    std::function<void(uint64_t)> blob_id_callback{};
};

struct hz_job {
    hz_codec_job *codec;
};

#endif
