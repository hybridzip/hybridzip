#ifndef HYBRIDZIP_ERRORS_MEMORY_H
#define HYBRIDZIP_ERRORS_MEMORY_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace MemoryErrors {
    class PeakLimitReachedException: public std::exception {
    public:
        PeakLimitReachedException() {
            LOG_F(ERROR, "hzip.memory: Peak limit reached");
        }

        const char *what() const throw() {
            return "hzip.memory: Peak limit reached";
        }
    };
}



#endif
