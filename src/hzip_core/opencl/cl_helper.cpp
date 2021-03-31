#include "cl_helper.h"
#include <iostream>
#include <regex>

#ifdef HZIP_ENABLE_OPENCL

#ifndef HZIP_OPENCL_BUILD_OPTIONS
#define HZIP_OPENCL_BUILD_OPTIONS "-cl-std=CL2.0"

using namespace hzopencl;

std::unordered_map<std::string, std::vector<cl::Program>> ProgramProvider::_program_map;
std::mutex ProgramProvider::_mutex;
uint64_t ProgramProvider::_device_index = 0;
uint64_t ProgramProvider::_device_count = 0;

std::vector<cl::Device> DeviceProvider::_devices;
std::vector<std::mutex> DeviceProvider::_mutexes;
std::mutex DeviceProvider::_mutex;

void DeviceProvider::list_available_devices() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::cout << "List of all available OpenCL devices:" << std::endl << std::endl;

    for (const auto &platform: platforms) {
        std::vector<cl::Device> platform_devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &platform_devices);

        std::cout << "Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

        for (const auto &device : platform_devices) {
            std::cout << "\t- Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            std::cout << "\t\t- Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz"
                      << std::endl;
            std::cout << "\t\t- Compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

std::vector<cl::Device> DeviceProvider::get_devices() {
    std::scoped_lock<std::mutex> lock(_mutex);
    if (_devices.empty()) {
        throw OpenCLErrors::InvalidOperationException("No devices were loaded");
    }

    return _devices;
}

bool DeviceProvider::empty() {
    std::scoped_lock<std::mutex> lock(_mutex);
    return _devices.empty();
}

void DeviceProvider::filter_devices(const std::string &regex_pattern) {
    std::scoped_lock<std::mutex> lock(_mutex);

    if (_devices.empty()) {
        throw OpenCLErrors::InvalidOperationException("No devices were loaded");
    }

    std::regex regex(regex_pattern);

    std::vector<cl::Device> filtered_devices;

    for (const auto &device : _devices) {
        std::string dname = device.getInfo<CL_DEVICE_NAME>();
        if (std::regex_search(dname.begin(), dname.end(), regex)) {
            filtered_devices.push_back(device);
        }
    }

    _devices = filtered_devices;

    ProgramProvider::clear();
    ProgramProvider::set_device_count(_devices.size());
}

void DeviceProvider::load_devices(uint32_t device_type) {
    std::scoped_lock<std::mutex> lock(_mutex);

    _devices.clear();
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto &platform: platforms) {
        std::vector<cl::Device> platform_devices;
        platform.getDevices(device_type, &platform_devices);
        _devices.insert(_devices.end(), platform_devices.begin(), platform_devices.end());
    }

    _mutexes = std::vector<std::mutex>(_devices.size());

    ProgramProvider::clear();
    ProgramProvider::set_device_count(_devices.size());
}

std::mutex &DeviceProvider::get_mutex(uint64_t index) {
    return _mutexes[index];
}

std::pair<cl::Program, std::mutex &> ProgramProvider::get(const std::string &program_name) {
    std::scoped_lock<std::mutex> lock(_mutex);

    if (!_program_map.contains(program_name)) {
        throw OpenCLErrors::InvalidOperationException("Failed to load unregistered OpenCL program");
    }

    auto program = _program_map[program_name][_device_index];
    auto output = std::pair<cl::Program, std::mutex &>(program, DeviceProvider::get_mutex(_device_index));

    _device_index = (_device_index + 1) % _device_count;
    return output;
}

void ProgramProvider::clear() {
    std::scoped_lock<std::mutex> lock(_mutex);

    _program_map.clear();
}

void ProgramProvider::register_program(const std::string &program_name, const std::string &src) {
    std::scoped_lock<std::mutex> lock(_mutex);

    if (!_program_map.contains(program_name)) {
        auto devices = DeviceProvider::get_devices();

        for (const auto &device : devices) {
            cl::Context context(device);
            auto program = cl::Program(context, src);
            try {
                program.build(HZIP_OPENCL_BUILD_OPTIONS);
            } catch (cl::Error &e) {
                if (e.err() == CL_BUILD_PROGRAM_FAILURE) {
                    cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device);
                    if (status == CL_BUILD_ERROR) {
                        std::string name = device.getInfo<CL_DEVICE_NAME>();
                        std::string build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
                        std::string err_msg = "Build failed for device: " + name;
                        err_msg += "\nBuild log - \n" + build_log;

                        throw OpenCLErrors::ProgramException(err_msg);
                    } else {
                        throw OpenCLErrors::InvalidOperationException(e.what());
                    }
                }
            }

            _program_map[program_name].push_back(program);
        }
    }
}

void ProgramProvider::set_device_count(uint64_t device_count) {
    std::scoped_lock<std::mutex> lock(_mutex);
    _device_count = device_count;
}

std::pair<cl::Kernel, std::mutex &> KernelProvider::get(const std::string &program_name) {
    auto[program, mutex] = ProgramProvider::get(program_name);
    return std::pair<cl::Kernel, std::mutex &>(cl::Kernel(program, "run"), mutex);
}

std::pair<cl::Kernel, std::mutex &> KernelProvider::get(const std::string &program_name, const std::string &kernel) {
    auto[program, mutex] = ProgramProvider::get(program_name);
    return std::pair<cl::Kernel, std::mutex &>(cl::Kernel(program, kernel.c_str()), mutex);
}

#endif
#endif