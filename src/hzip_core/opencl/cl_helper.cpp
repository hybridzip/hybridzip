#include "cl_helper.h"
#include <iostream>

#ifdef HZIP_ENABLE_OPENCL

#ifndef HZIP_OPENCL_BUILD_OPTIONS
#define HZIP_OPENCL_BUILD_OPTIONS "-cl-std=CL2.0"

using namespace hzopencl;

std::unordered_map<std::string, cl::Program> ProgramProvider::_program_map;
std::unordered_map<std::string, std::string> ProgramProvider::_src_map;
std::mutex ProgramProvider::_mutex;

std::vector<cl::Device> DeviceProvider::_devices;
std::mutex DeviceProvider::_mutex;
uint64_t DeviceProvider::_device_index = 0;
std::string DeviceProvider::_preferred_device_name;

std::mutex Runtime::mutex;

void DeviceProvider::list_available_devices() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::cout << "List of all available OpenCL devices:" << std::endl << std::endl;

    for (const auto &platform: platforms) {
        std::vector<cl::Device> platform_devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &platform_devices);

        std::cout << "Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

        for (const auto& device : platform_devices) {
            std::cout << "\t- Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            std::cout << "\t\t- Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz" << std::endl;
            std::cout << "\t\t- Compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
            std::cout << std::endl;
        }
        std::cout << std::endl;

        _devices.insert(_devices.end(), platform_devices.begin(), platform_devices.end());
    }
}

cl::Device DeviceProvider::get() {
    _mutex.lock();
    if (_devices.empty()) {
        _mutex.unlock();
        throw OpenCLErrors::InvalidOperationException("No devices were loaded");
    }
    cl::Device device = _devices[_device_index];
    if (_preferred_device_name.empty()) {
        _device_index = (_device_index + 1) % _devices.size();
    }
    _mutex.unlock();
    return device;
}

bool DeviceProvider::empty() {
    _mutex.lock();
    bool v = _devices.empty();
    _mutex.unlock();
    return v;
}

void DeviceProvider::set_preferred_device(const std::string &dev_name) {
    _preferred_device_name = dev_name;
    bool not_found_device = true;
    _mutex.lock();
    if (_devices.empty()) {
        _mutex.unlock();
        throw OpenCLErrors::InvalidOperationException("No devices were loaded");
    }
    for (int i = 0; i < _devices.size(); i++) {
        auto device = _devices[i];
        std::string dname = device.getInfo<CL_DEVICE_NAME>();

        if (dname.find(_preferred_device_name) != std::string::npos) {
            not_found_device = false;
            _device_index = i;
            break;
        }
    }

    if (not_found_device) {
        _preferred_device_name = "";
    }

    _mutex.unlock();
}

void DeviceProvider::load_devices(uint32_t device_type) {
    _mutex.lock();
    _devices.clear();
    _device_index = 0;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto &platform: platforms) {
        std::vector<cl::Device> platform_devices;
        platform.getDevices(device_type, &platform_devices);
        _devices.insert(_devices.end(), platform_devices.begin(), platform_devices.end());
    }

    ProgramProvider::clear();
    _mutex.unlock();
}

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

void ProgramProvider::register_program(const std::string &program_name, const std::string &src) {
    _mutex.lock();
    if (!_program_map.contains(program_name)) {
        cl::Context context(DeviceProvider::get());
        auto program = cl::Program(context, src);
        program.build(HZIP_OPENCL_BUILD_OPTIONS);
        _program_map[program_name] = program;
        _src_map[program_name] = src;
    }
    _mutex.unlock();
}

void ProgramProvider::compile(const std::string &program_name, const cl::Device &device) {
    _mutex.lock();
    if (!_program_map.contains(program_name)) {
        throw OpenCLErrors::InvalidOperationException("Cannot set device for unregistered OpenCL program");
    } else {
        cl::Context context(device);
        auto program = cl::Program(context, _src_map[program_name]);
        program.build(HZIP_OPENCL_BUILD_OPTIONS);
        _program_map[program_name] = program;
    }
    _mutex.unlock();
}

cl::Kernel KernelProvider::get(const std::string &program_name) {
    cl::Program program = ProgramProvider::get(program_name);
    return cl::Kernel(program, "run");
}

cl::Kernel KernelProvider::get(const std::string &program_name, const std::string &kernel) {
    cl::Program program = ProgramProvider::get(program_name);
    return cl::Kernel(program, kernel.c_str());
}

#endif
#endif