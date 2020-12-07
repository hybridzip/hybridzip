#ifndef HYBRIDZIP_ERRORS_UTILS_H
#define HYBRIDZIP_ERRORS_UTILS_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace UtilErrors {
    class ValidationError : public std::exception {
    public:

        ValidationError(const std::string &msg) {
            LOG_F(ERROR, "hzip.utils.validation: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Validation error occured";
        }
    };

    class InternalError : std::exception {
    public:

        InternalError(const std::string &msg) {
            LOG_F(ERROR, "hzip: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Internal error occured";
        }
    };
}

#endif
