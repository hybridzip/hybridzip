#ifndef HYBRIDZIP_ERRORS_PROCESSOR_H
#define HYBRIDZIP_ERRORS_PROCESSOR_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace ProcessorErrors {
    class InvalidOperationError : public std::exception {
    private:
        std::string _msg;
    public:
        InvalidOperationError(const std::string &msg) {
            LOG_F(ERROR, "hzip_core.processor: %s", msg.c_str());
            _msg = "hzip_core.processor: " + msg;
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
            LOG_F(ERROR, "hzip_core.processor: %s", _msg.c_str());
            msg = _msg;
        }

        const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}

#endif
