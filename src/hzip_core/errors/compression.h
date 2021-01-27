#ifndef HYBRIDZIP_COMPRESSION_H
#define HYBRIDZIP_COMPRESSION_H

#include <exception>
#include <string>

namespace CompressionErrors {
    class InvalidOperationException : public std::exception {
    public:
        std::string msg;

        InvalidOperationException(const std::string &msg) {
            this->msg = "hzip_core.core: Invalid operation: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}

#endif
