#ifndef HYBRIDZIP_CODEC_H
#define HYBRIDZIP_CODEC_H

#include <exception>
#include <string>

namespace CodecErrors {
    class InvalidOperationException : public std::exception {
    public:
        std::string msg;

        InvalidOperationException(const std::string &msg) {
            this->msg = "hzip_codec: Invalid operation: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class ResourceResolutionException : public std::exception {
    public:
        std::string msg;

        ResourceResolutionException(const std::string &msg) {
            this->msg = "hzip_codec: Resource resolution failed: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}

#endif
