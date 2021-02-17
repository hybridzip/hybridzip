#include "color.h"
#include <hzip_core/opencl/cl_helper.h>
#include <hzip_core/config.h>

hztrans::LinearU8ColorTransformer::LinearU8ColorTransformer(
        uint64_t width,
        uint64_t height
) : _width(width), _height(height), _executor(get_best_executor()) {
    if (Config::opencl_support_enabled) {
        if (width * height > 1000000) {
            _executor = OPENCL;
        } else {
            _executor = CPU;
        }
    }
}

rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) {
    if (_executor == OPENCL) {
        return opencl_rgb_to_ycocg(buffer);
    } else {
        return cpu_rgb_to_ycocg(buffer);
    }
}

rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::cpu_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) const {
    auto output = rainman::ptr<uint8_t>(buffer.size());

    auto transformer = RGBColorTransformer<uint8_t>(HZ_COLOR_TRANSFORM::RGB_TO_YCOCG);
    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto r = buffer[i];
        auto g = buffer[i + lz];
        auto b = buffer[i + lz2];

        auto pixel = ColorTransformPixel<uint8_t>{r, g, b};
        pixel = transformer.transform(pixel);

        output[i] = pixel.x;
        output[i + lz] = pixel.z;
        output[i + lz2] = pixel.y;
    }

    return output;
}


rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::ycocg_to_rgb(const rainman::ptr<uint8_t> &buffer) {
    if (_executor == OPENCL) {
        return opencl_ycocg_to_rgb(buffer);
    } else {
        return cpu_ycocg_to_rgb(buffer);
    }
}

rainman::ptr<uint8_t> hztrans::LinearU8ColorTransformer::cpu_ycocg_to_rgb(const rainman::ptr<uint8_t> &buffer) const {
    auto output = rainman::ptr<uint8_t>(buffer.size());

    auto transformer = RGBColorTransformer<uint8_t>(HZ_COLOR_TRANSFORM::YCOCG_TO_RGB);
    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto y = buffer[i];
        auto co = buffer[i + lz];
        auto cg = buffer[i + lz2];

        auto pixel = ColorTransformPixel<uint8_t>{y, co, cg};
        pixel = transformer.transform(pixel);

        output[i] = pixel.x;
        output[i + lz] = pixel.y;
        output[i + lz2] = pixel.z;
    }

    return output;
}


#ifdef HZIP_ENABLE_OPENCL

rainman::ptr<uint8_t>
hztrans::LinearU8ColorTransformer::opencl_rgb_to_ycocg(const rainman::ptr<uint8_t> &buffer) const {
    register_kernel();

    auto kernel = hzopencl::KernelProvider::get("rgb_ycocg", "rgb_to_ycocg");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = n / Config::opencl_kernels;
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint8_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);

    auto output = rainman::ptr<uint8_t>(buffer.size());

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint8_t), output.pointer());

    queue.finish();

    return output;
}


void hztrans::LinearU8ColorTransformer::register_kernel() {
    hzopencl::ProgramProvider::register_program("rgb_ycocg",

#include "rgb_ycocg.cl"

    );
}

rainman::ptr<uint8_t>
hztrans::LinearU8ColorTransformer::opencl_ycocg_to_rgb(const rainman::ptr<uint8_t> &buffer) const {
    register_kernel();

    auto kernel = hzopencl::KernelProvider::get("rgb_ycocg", "ycocg_to_rgb");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = n / Config::opencl_kernels;
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint8_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);

    auto output = rainman::ptr<uint8_t>(buffer.size());

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint8_t), output.pointer());

    queue.finish();

    return output;
}

#endif

rainman::ptr<uint16_t>
hztrans::LinearU16ColorTransformer::cpu_rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer) const {
    auto output = rainman::ptr<uint16_t>(buffer.size());

    auto transformer = RGBColorTransformer<uint16_t>(HZ_COLOR_TRANSFORM::RGB_TO_YCOCG);
    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto r = buffer[i];
        auto g = buffer[i + lz];
        auto b = buffer[i + lz2];

        auto pixel = ColorTransformPixel<uint16_t>{r, g, b};
        pixel = transformer.transform(pixel);

        output[i] = pixel.x;
        output[i + lz] = pixel.z;
        output[i + lz2] = pixel.y;
    }

    return output;
}

rainman::ptr<uint16_t>
hztrans::LinearU16ColorTransformer::cpu_ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer) const {
    auto output = rainman::ptr<uint16_t>(buffer.size());

    auto transformer = RGBColorTransformer<uint16_t>(HZ_COLOR_TRANSFORM::YCOCG_TO_RGB);
    uint64_t lz = _width * _height;
    uint64_t lz2 = lz << 1;

    for (uint64_t i = 0; i < lz; i++) {
        auto y = buffer[i];
        auto co = buffer[i + lz];
        auto cg = buffer[i + lz2];

        auto pixel = ColorTransformPixel<uint16_t>{y, co, cg};
        pixel = transformer.transform(pixel);

        output[i] = pixel.x;
        output[i + lz] = pixel.y;
        output[i + lz2] = pixel.z;
    }

    return output;
}

hztrans::LinearU16ColorTransformer::LinearU16ColorTransformer(
        uint64_t width,
        uint64_t height
) : _width(width), _height(height), _executor(get_best_executor()) {
    if (Config::opencl_support_enabled) {
        if (width * height > 1000000) {
            _executor = OPENCL;
        } else {
            _executor = CPU;
        }
    }
}

rainman::ptr<uint16_t> hztrans::LinearU16ColorTransformer::rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer) {
    if (_executor == OPENCL) {
        return opencl_rgb_to_ycocg(buffer);
    } else {
        return cpu_rgb_to_ycocg(buffer);
    }
}

rainman::ptr<uint16_t> hztrans::LinearU16ColorTransformer::ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer) {
    if (_executor == OPENCL) {
        return opencl_ycocg_to_rgb(buffer);
    } else {
        return cpu_ycocg_to_rgb(buffer);
    }
}

#ifdef HZIP_ENABLE_OPENCL

void hztrans::LinearU16ColorTransformer::register_kernel() {
    hzopencl::ProgramProvider::register_program("rgb_ycocg",

#include "rgb_ycocg.cl"

    );
}

rainman::ptr<uint16_t>
hztrans::LinearU16ColorTransformer::opencl_rgb_to_ycocg(const rainman::ptr<uint16_t> &buffer) const {
    register_kernel();

    auto kernel = hzopencl::KernelProvider::get("rgb_ycocg", "rgb_to_ycocg16");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = n / Config::opencl_kernels;
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint16_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);

    auto output = rainman::ptr<uint16_t>(buffer.size());

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), output.pointer());

    queue.finish();

    return output;
}

rainman::ptr<uint16_t>
hztrans::LinearU16ColorTransformer::opencl_ycocg_to_rgb(const rainman::ptr<uint16_t> &buffer) const {
    register_kernel();

    auto kernel = hzopencl::KernelProvider::get("rgb_ycocg", "ycocg_to_rgb16");
    auto context = kernel.getInfo<CL_KERNEL_CONTEXT>();
    auto device = context.getInfo<CL_CONTEXT_DEVICES>().front();

    uint64_t local_size = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);

    uint64_t n = _width * _height;
    uint64_t stride_size = n / Config::opencl_kernels;
    uint64_t true_size = (n / stride_size) + (n % stride_size != 0);
    uint64_t global_size = (true_size / local_size + (true_size % local_size != 0)) * local_size;

    cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   buffer.size() * sizeof(uint16_t), buffer.pointer());

    kernel.setArg(0, buf);
    kernel.setArg(1, n);
    kernel.setArg(2, stride_size);

    auto output = rainman::ptr<uint16_t>(buffer.size());

    auto queue = cl::CommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(global_size), cl::NDRange(local_size));
    queue.enqueueBarrierWithWaitList();

    queue.enqueueReadBuffer(buf, CL_FALSE, 0, buffer.size() * sizeof(uint16_t), output.pointer());

    queue.finish();

    return output;
}

#endif