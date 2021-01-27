#ifndef HYBRIDZIP_JOB_H
#define HYBRIDZIP_JOB_H

#include <cstdint>
#include <functional>
#include <hzip_storage/archive/archive.h>
#include <hzip_core/core/blob/blob.h>

struct HZ_CodecJob {
    enum CodecJobType {
        ENCODE = 0x0,
        DECODE = 0x1,
        TRAIN = 0x2,
    };

    rainman::ptr<HZ_Blob> blob;
    rainman::option<rainman::ptr<HZ_Archive>> archive;
    hzcodec::algorithms::ALGORITHM algorithm;
    CodecJobType job_type;
    rainman::option<std::string> mstate_addr;
    std::function<void(uint64_t)> blob_id_callback{};
    std::function<void(rainman::ptr<HZ_Blob>)> blob_callback{};
};

struct HZ_JobStub {
    std::function<void()> on_completed{};
    std::function<void(const std::string &)> on_error{};
    std::function<void(const std::string &)> on_success{};
};

struct HZ_Job {
    enum JobType {
        CODEC = 0x0,
    };

    JobType type;
    rainman::ptr<HZ_CodecJob> codec;
    rainman::ptr<HZ_JobStub> stub;
};

#endif
