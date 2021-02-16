#ifndef HZIP_CODEC_EXECUTOR_H
#define HZIP_CODEC_EXECUTOR_H

#include <hzip_codec/opencl/cl_helper.h>
#include "executor.h"

enum Executor {
    CPU,
    OPENCL
};

inline Executor get_best_executor() {
#ifdef HZIP_ENABLE_OPENCL
    if (hzcodec::opencl::DeviceProvider::empty()) {
        return Executor::CPU;
    } else {
        return Executor::OPENCL;
    }
#endif
    return Executor::CPU;
}

#endif
