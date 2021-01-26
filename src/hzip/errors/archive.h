#ifndef HYBRIDZIP_ERRORS_ARCHIVE_H
#define HYBRIDZIP_ERRORS_ARCHIVE_H

#include <exception>
#include <loguru/include/loguru/loguru.hpp>
#include <cstdint>

namespace ArchiveErrors {
    class BlobNotFoundException : public std::exception {
    public:
        uint64_t id{};

        BlobNotFoundException(uint64_t id) {
            LOG_F(ERROR, "hzip.archive: Blob(0x%lx) was not found", id);
            this->id = id;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.archive: Blob not found";
        }
    };

    class FileNotFoundException : public std::exception {
    public:

        FileNotFoundException(const std::string &path) {
            LOG_F(ERROR, "hzip.archive: File (%s) was not found", path.c_str());
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.archive: File not found";
        }
    };

    class MstateNotFoundException : public std::exception {
    public:
        uint64_t id{};

        MstateNotFoundException(uint64_t id) {
            LOG_F(ERROR, "hzip.archive: Mstate(0x%lx) was not found", id);
            this->id = id;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.archive: Mstate not found";
        }
    };

    class InvalidOperationException : public std::exception {
    public:
        std::string msg;

        InvalidOperationException(const std::string &msg) {
            this->msg = "hzip.archive: Invalid operation: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class TargetNotFoundException : public std::exception {
    public:
        std::string msg;

        TargetNotFoundException(const std::string &msg) {
            this->msg = "hzip.archive: Target not found: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };
}
#endif
