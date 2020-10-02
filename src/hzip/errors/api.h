#ifndef HYBRIDZIP_ERRORS_API_H
#define HYBRIDZIP_ERRORS_API_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace ApiErrors {
    class InitializationError: public std::exception {
    public:

        InitializationError(const std::string& msg) {
            LOG_F(ERROR, "hzip.api: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Initialization error occured";
        }
    };
}

#endif
