#include "cl_helper.h"
#include <hzip_core/errors/opencl.h>

#ifdef HZIP_ENABLE_OPENCL
#ifndef HZIP_OPENCL_BUILD_OPTIONS
#define HZIP_OPENCL_BUILD_OPTIONS "-cl-std=CL2.0"
#endif

using namespace hzcodec::opencl;

std::unordered_map<std::string, cl::Program> ProgramProvider::_program_map;
std::unordered_map<std::string, std::string> ProgramProvider::_src_map;
std::mutex ProgramProvider::_mutex;

std::vector<cl::Device> DeviceProvider::_devices;
std::mutex DeviceProvider::_mutex;
uint64_t DeviceProvider::_device_index = 0;


cl::Program ProgramProvider::get(const std::string &kernel) {
    _mutex.lock();
    if (!_program_map.contains(kernel)) {
        _mutex.unlock();
        throw OpenCLErrors::InvalidOperationException("Failed to load unregistered OpenCL kernel");
    }
    _mutex.unlock();
    return _program_map[kernel];
}

void ProgramProvider::clear() {
    _mutex.lock();
    _program_map.clear();
    _mutex.unlock();
}

void ProgramProvider::register_kernel(const std::string &kernel, const std::string &src) {
    _mutex.lock();
    if (!_program_map.contains(kernel)) {
        cl::Context context(DeviceProvider::get());
        auto program = cl::Program(context, src);
        program.build(HZIP_OPENCL_BUILD_OPTIONS);
        _program_map[kernel] = program;
        _src_map[kernel] = src;
    }
    _mutex.unlock();
}

void ProgramProvider::compile(const std::string &kernel, const cl::Device &device) {
    _mutex.lock();
    if (!_program_map.contains(kernel)) {
        throw OpenCLErrors::InvalidOperationException("Cannot set device for unregistered OpenCL kernel");
    } else {
        cl::Context context(device);
        auto program = cl::Program(context, _src_map[kernel]);
        program.build(HZIP_OPENCL_BUILD_OPTIONS);
        _program_map[kernel] = program;
    }
    _mutex.unlock();
}

cl::Kernel KernelProvider::get(const std::string &kernel) {
    cl::Program program = ProgramProvider::get(kernel);
    return cl::Kernel(program, "run");
}

cl::Kernel KernelProvider::get(const std::string &kernel, const std::string &name) {
    cl::Program program = ProgramProvider::get(kernel);
    return cl::Kernel(program, name.c_str());
}

#endif