#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>
#include <functional>
#include <hzip/archive/archive.h>
#include <hzip/core/blob/blob.h>

struct HZ_CodecJob {
    enum JOBTYPE {
        ENCODE = 0x0,
        DECODE = 0x1,
        TRAIN = 0x2,
    };

    HZ_Blob *blob;
    HZ_Archive *archive;
    hzcodec::algorithms::ALGORITHM algorithm;
    JOBTYPE job_type;
    bool use_mstate_addr;
    std::string mstate_addr;
    std::function<void(uint64_t)> blob_id_callback{};
    std::function<void(HZ_Blob *)> blob_callback{};
};

struct HZ_JobStub {
    std::function<void()> on_completed{};
    std::function<void(const std::string &)> on_error{};
    std::function<void(const std::string &)> on_success{};
};

struct HZ_Job {
    HZ_CodecJob *codec;
    HZ_JobStub *stub;
};

#endif
