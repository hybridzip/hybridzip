#include <hzip_core/config.h>
#include <hzip_core/executor.h>
#include <hzip_core/opencl/cl_helper.h>
#include <hzip_core/runtime.h>
#include "paeth.h"

rainman::ptr<uint16_t>
hzmodels::LinearU16PaethDifferential::filter(
        const rainman::ptr<uint16_t> &buffer,
        uint64_t width,
        uint64_t height,
        uint64_t nchannels,
        bool inplace
) {
    Executor executor = CPU;
    if (Config::opencl_support_enabled) {
        if (buffer.size() > 49152) {
            executor = OPENCL;
        } else {
            executor = CPU;
        }
    }

    if (executor == OPENCL) {
        return opencl_filter(buffer, width, height, nchannels, inplace);
    } else {
        return cpu_filter(buffer, width, height, nchannels, inplace);
    }
}

rainman::ptr<uint16_t>
hzmodels::LinearU16PaethDifferential::cpu_filter(
        const rainman::ptr<uint16_t> &buffer,
        uint64_t width,
        uint64_t height,
        uint64_t nchannels,
        bool inplace
) {
    rainman::ptr<uint16_t> output;

    if (inplace) {
        output = buffer;
    } else {
        output = rainman::ptr<uint16_t>(buffer.size());
    }

    for (uint64_t channel_index = 0; channel_index < nchannels; channel_index++) {
        uint64_t channel_offset = channel_index * width * height;
        for (int32_t y = height - 1; y >= 0; y--) {
            uint64_t offset = channel_offset + y * width;
            for (int32_t x = width - 1; x >= 0; x--) {
                if (y > 0 && x > 0) {
                    uint32_t t = buffer[offset + x];
                    uint32_t a = buffer[offset + x - 1];
                    uint32_t b = buffer[offset + x - width];
                    uint32_t c = buffer[offset + x - width - 1];

                    uint32_t d = a + b - c;
                    uint32_t da = absdiff(a, d);
                    uint32_t db = absdiff(b, d);
                    uint32_t dc = absdiff(c, d);
                    uint32_t dd = min3(da, db, dc);

                    if (da == dd) {
                        d = a;
                    } else if (db == dd) {
                        d = b;
                    } else {
                        d = c;
                    }

                    uint16_t diff = t | d;
                    output[offset + x] = diff;
                    continue;
                }

                if (y == 0 && x > 0) {
                    uint16_t t = buffer[offset + x];
                    uint16_t d = buffer[offset + x - 1];

                    uint16_t diff = t | d;
                    output[offset + x] = diff;
                    continue;
                }

                if (y > 0 && x == 0) {
                    uint16_t t = buffer[offset + x];
                    uint16_t d = buffer[offset + x - width];

                    uint16_t diff = t | d;
                    output[offset + x] = diff;
                    continue;
                }

                if (y == 0 && x == 0) {
                    output[offset + x] = buffer[offset + x];
                }
            }
        }
    }

    return output;
}

#ifdef HZIP_ENABLE_OPENCL

void hzmodels::LinearU16PaethDifferential::register_opencl_program() {
    hzopencl::ProgramProvider::register_program("paeth_differential",

#include "paeth.cl"

    );
}

rainman::ptr<uint16_t>
hzmodels::LinearU16PaethDifferential::opencl_filter(
        const rainman::ptr<uint16_t> &buffer,
        uint64_t width,
        uint64_t height,
        uint64_t nchannels,
        bool inplace
) {
    register_opencl_program();

    rainman::ptr<uint16_t> output;
    if (inplace) {
        output = buffer;
    } else {
        output = rainman::ptr<uint16_t>(buffer.size());
    }

    for (uint64_t channel_index = 0; channel_index < nchannels; channel_index++) {
        uint64_t channel_offset = channel_index * width * height;
        auto kernel = hzopencl::KernelProvider::get("paeth_differential", "paeth_differential16");
        auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
        auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

        uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

        uint64_t n = width * height;
        uint64_t stride_size = (n / Config::opencl_kernels) + (n % Config::opencl_kernels != 0);
        uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
        uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

        cl::Buffer buf_input(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             n * sizeof(uint16_t), buffer.pointer() + channel_offset);

        cl::Buffer buf_output(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, n * sizeof(uint16_t));

        kernel.setArg(0, buf_input);
        kernel.setArg(1, buf_output);
        kernel.setArg(2, width);
        kernel.setArg(3, height);
        kernel.setArg(4, n);
        kernel.setArg(5, stride_size);

        std::unique_lock<std::mutex> lock(HZRuntime::opencl_mutex);

        auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
        queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
        queue.enqueueBarrierWithWaitList();

        queue.enqueueReadBuffer(buf_output, CL_FALSE, 0, n * sizeof(uint16_t), output.pointer() + channel_offset);
        queue.finish();
    }

    return output;
}

#endif
