#ifndef HZIP_CORE_CL_HELPER_H
#define HZIP_CORE_CL_HELPER_H

#ifdef HZIP_ENABLE_OPENCL

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <CL/opencl.hpp>
#include <hzip_core/errors/opencl.h>

namespace hzopencl {
    class ProgramProvider {
    private:
        static std::unordered_map<std::string, std::vector<cl::Program>> _program_map;
        static std::mutex _mutex;
        static uint64_t _device_index;
        static uint64_t _device_count;

    public:
        static std::pair<cl::Program, std::mutex &> get(const std::string &program_name);

        static void register_program(const std::string &program_name, const std::string &src);

        static void clear();

        static void set_device_count(uint64_t device_count);
    };

    class DeviceProvider {
    private:
        static std::vector<cl::Device> _devices;
        static std::vector<std::mutex> _mutexes;
        static std::mutex _mutex;
    public:
        static void load_devices(uint32_t device_type = CL_DEVICE_TYPE_ALL);

        static void list_available_devices();

        static std::vector<cl::Device> get_devices();

        static std::mutex &get_mutex(uint64_t index);

        static bool empty();

        static void filter_devices(const std::string &regex_pattern);
    };

    class KernelProvider {
    public:
        static std::pair<cl::Kernel, std::mutex &> get(const std::string &program_name);

        static std::pair<cl::Kernel, std::mutex &> get(const std::string &program_name, const std::string &kernel);
    };
}

#endif
#endif