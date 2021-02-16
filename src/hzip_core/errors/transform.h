#ifndef HYBRIDZIP_ERRORS_TRANSFORM_H
#define HYBRIDZIP_ERRORS_TRANSFORM_H

#include <loguru/loguru.hpp>
#include <exception>
#include <string>

namespace TransformErrors {
    class InvalidInputError : public std::exception {
    public:

        InvalidInputError(const std::string &msg) {
            LOG_F(ERROR, "hzip_core.preprocessor: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Invalid input error";
        }
    };

    class InvalidOperationError : public std::exception {
    public:

        InvalidOperationError(const std::string &msg) {
            LOG_F(ERROR, "hzip_core.preprocessor: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Invalid operation error";
        }
    };
}

#endif
