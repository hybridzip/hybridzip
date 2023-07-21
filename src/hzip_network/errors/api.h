#ifndef HYBRIDZIP_ERRORS_API_H
#define HYBRIDZIP_ERRORS_API_H

#include <exception>
#include <string>
#include <loguru/loguru.hpp>

namespace ApiErrors {
    class InitializationError : public std::exception {
    public:

        InitializationError(const std::string &msg) {
            LOG_F(ERROR, "hzip_core.api: %s", msg.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "Initialization error occured";
        }
    };

    class InvalidOperationError : public std::exception {
    private:
        std::string _msg;
    public:
        InvalidOperationError(const std::string &msg) {
            _msg = msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return _msg.c_str();
        }
    };

    class ConnectionError : public std::exception {
    private:
        std::string _msg;
    public:
        ConnectionError(const std::string &msg) {
            _msg = msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return _msg.c_str();
        }
    };
}

#endif
