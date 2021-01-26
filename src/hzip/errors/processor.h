#ifndef HYBRIDZIP_ERRORS_PROCESSOR_H
#define HYBRIDZIP_ERRORS_PROCESSOR_H

#include <exception>
#include <string>
#include <loguru/include/loguru/loguru.hpp>

namespace ProcessorErrors {
    class InvalidOperationError : public std::exception {
    private:
        std::string _msg;
    public:
        InvalidOperationError(const std::string &msg) {
            LOG_F(ERROR, "hzip.processor: %s", msg.c_str());
            _msg = "hzip.processor: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return _msg.c_str();
        }
    };

    class GenericError : public std::exception {
    private:
        std::string msg;
    public:
        GenericError(const std::string &_msg) {
            msg = _msg;
        }

        const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}

#endif
