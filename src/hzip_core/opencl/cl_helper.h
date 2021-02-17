#ifndef HZIP_CORE_CL_HELPER_H
#define HZIP_CORE_CL_HELPER_H

#ifdef HZIP_ENABLE_OPENCL

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <CL/opencl.hpp>
#include <hzip_core/errors/opencl.h>

namespace hzopencl {
    class ProgramProvider {
    private:
        static std::unordered_map<std::string, cl::Program> _program_map;
        static std::unordered_map<std::string, std::string> _src_map;
        static std::mutex _mutex;
    public:
        static cl::Program get(const std::string &kernel);

        static void register_kernel(const std::string &kernel, const std::string &src);

        static void compile(const std::string &kernel, const cl::Device &device);

        static void clear();
    };

    class DeviceProvider {
    private:
        static std::vector<cl::Device> _devices;
        static std::mutex _mutex;
        static uint64_t _device_index;
        static std::string _preferred_device_name;
    public:
        static void load_devices(uint32_t device_type = CL_DEVICE_TYPE_ALL);

        static void list_available_devices();

        static cl::Device get();

        static bool empty();

        static void set_preferred_device(const std::string &dev_name);
    };

    class KernelProvider {
    public:
        static cl::Kernel get(const std::string &kernel);

        static cl::Kernel get(const std::string &kernel, const std::string &name);
    };
}

#endif
#endif