#ifndef HYBRIDZIP_ERRORS_ARCHIVE_H
#define HYBRIDZIP_ERRORS_ARCHIVE_H

#include <exception>
#include <loguru/loguru.hpp>
#include <cstdint>

namespace ArchiveErrors {
    class BlobNotFoundException: public std::exception {
    public:
        uint64_t id{};

        BlobNotFoundException(uint64_t id) {
            LOG_F(ERROR, "hzip.archive: blob(0x%lx) was not found", id);
            this->id = id;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.archive: blob not found";
        }
    };

    class MstateNotFoundException: public std::exception {
    public:
        uint64_t id{};

        MstateNotFoundException(uint64_t id) {
            LOG_F(ERROR, "hzip.archive: mstate(0x%lx) was not found", id);
            this->id = id;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return "hzip.mstate: blob not found";
        }
    };
}
#endif
