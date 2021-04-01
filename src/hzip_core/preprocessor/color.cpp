#include "color.h"
#include <hzip_core/opencl/cl_helper.h>
#include <hzip_core/runtime/runtime.h>

hztrans::LinearU16XColorTransformer::LinearU16XColorTransformer(
        uint64_t width,
        uint64_t height,
        uint8_t bit_depth
) : _width(width), _height(height), _bit_depth(bit_depth) {
    _mask = (1ull << _bit_depth) - 1;
    if (hzruntime::Config::opencl_support_enabled) {
        if (_width * _height > 1000000) {
            _executor = hzruntime::OPENCL;
        } else {
            _executor = hzruntime::CPU;
        }
    }
}

std::pair<uint16_t, uint16_t>
hztrans::LinearU16XColorTransformer::forward_lift(const std::pair<uint16_t, uint16_t> &p) const {
    auto[x, y] = p;
    x &= _mask;
    y &= _mask;

    uint16_t diff = (y - x) & _mask;
    uint16_t average = (x + (diff >> 1)) & _mask;

    return std::pair(average, diff);
}

std::pair<uint16_t, uint16_t>
hztrans::LinearU16XColorTransformer::reverse_lift(const std::pair<uint16_t, uint16_t> &p) const {
    auto[average, diff] = p;
    average &= _mask;
    diff &= _mask;

    uint16_t x = (average - (diff >> 1)) & _mask;
    uint16_t y = (x + diff) & _mask;

    return std::pair(x, y);
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::cpu_rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer, bool inplace) const {
    rainman::ptr<uint16_t> output;

    if (inplace) {
        output = buffer;
    } else {
        output = rainman::ptr<uint16_t>(buffer.size());
    }

    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto r = buffer[i];
        auto g = buffer[i + lz];
        auto b = buffer[i + lz2];

        auto[temp, co] = forward_lift(std::pair(r, b));
        auto[y, cg] = forward_lift(std::pair(g, temp));

        output[i] = y;
        output[i + lz] = co;
        output[i + lz2] = cg;
    }

    return output;
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::cpu_ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer, bool inplace) const {
    rainman::ptr<uint16_t> output;

    if (inplace) {
        output = buffer;
    } else {
        output = rainman::ptr<uint16_t>(buffer.size());
    }

    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto y = buffer[i];
        auto co = buffer[i + lz];
        auto cg = buffer[i + lz2];

        auto[g, temp] = reverse_lift(std::pair(y, cg));
        auto[r, b] = reverse_lift(std::pair(temp, co));

        output[i] = r;
        output[i + lz] = g;
        output[i + lz2] = b;
    }

    return output;
}

void hztrans::LinearU16XColorTransformer::register_opencl_program() {
    hzopencl::ProgramProvider::register_program("rgb_ycocg",

#include "hzip_core/opencl/common/types.cl"
#include "hzip_core/opencl/common/extended_types.cl"
#include "rgb_ycocg.cl"

    );
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::opencl_rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer, bool inplace) const {
    register_opencl_program();

    auto[kernel, device_mutex] = hzopencl::KernelProvider::get("rgb_ycocg", "u16x_rgb_to_ycocg");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = (n / hzruntime::Config::opencl_kernels) + (n % hzruntime::Config::opencl_kernels != 0);
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint16_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);
    kernel.setArg(3, _mask);


    std::scoped_lock<std::mutex> lock(device_mutex);

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    if (inplace) {
        queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), buffer.pointer());
        queue.finish();
        return buffer;
    } else {
        auto output = rainman::ptr<uint16_t>(buffer.size());
        queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), output.pointer());
        queue.finish();
        return output;
    }
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::opencl_ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer, bool inplace) const {
    register_opencl_program();

    auto[kernel, device_mutex] = hzopencl::KernelProvider::get("rgb_ycocg", "u16x_ycocg_to_rgb");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = (n / hzruntime::Config::opencl_kernels) + (n % hzruntime::Config::opencl_kernels != 0);
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint16_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);
    kernel.setArg(3, _mask);


    std::scoped_lock<std::mutex> lock(device_mutex);

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    if (inplace) {
        queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), buffer.pointer());
        queue.finish();
        return buffer;
    } else {
        auto output = rainman::ptr<uint16_t>(buffer.size());
        queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), output.pointer());
        queue.finish();
        return output;
    }
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer, bool inplace) {
    if (_executor == hzruntime::OPENCL) {
        return opencl_rgb_to_ycocg(buffer, inplace);
    } else {
        return cpu_rgb_to_ycocg(buffer, inplace);
    }
}

rainman::ptr<uint16_t>
hztrans::LinearU16XColorTransformer::ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer, bool inplace) {
    if (_executor == hzruntime::OPENCL) {
        return opencl_ycocg_to_rgb(buffer, inplace);
    } else {
        return cpu_ycocg_to_rgb(buffer, inplace);
    }
}
