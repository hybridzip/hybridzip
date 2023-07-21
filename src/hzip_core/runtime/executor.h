#ifndef HZIP_CORE_RUNTIME_EXECUTOR_H
#define HZIP_CORE_RUNTIME_EXECUTOR_H

#include <hzip_core/opencl/cl_helper.h>

namespace hzruntime {
    enum Executor {
        CPU,
        OPENCL
    };

    inline Executor get_best_executor() {
#ifdef HZIP_ENABLE_OPENCL
        if (hzopencl::DeviceProvider::empty()) {
            return Executor::CPU;
        } else {
            return Executor::OPENCL;
        }
#endif
        return Executor::CPU;
    }
}

#endif
