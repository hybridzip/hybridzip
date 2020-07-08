#ifndef HYBRIDZIP_ERRORS_MEMORY_H
#define HYBRIDZIP_ERRORS_MEMORY_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace MemoryErrors {
    class PeakLimitReachedException: public std::exception {
    public:
        PeakLimitReachedException() {
            LOG_F(ERROR, "hzip.memory: peak limit reached");
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.memory: peak limit reached";
        }
    };
}



#endif
