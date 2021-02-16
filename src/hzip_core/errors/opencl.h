#ifndef HZIP_CODEC_ERRORS_OPENCL_H
#define HZIP_CODEC_ERRORS_OPENCL_H

namespace OpenCLErrors {
    class InvalidOperationException : public std::exception {
    public:
        std::string msg;

        InvalidOperationException(const std::string &msg) {
            this->msg = "hzip_codec.opencl: Invalid operation: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}
#endif