#ifndef HYBRIDZIP_BLOB_H
#define HYBRIDZIP_BLOB_H

#include <cstdint>
#include <rainman/rainman.h>
#include <hzip/utils/platform.h>
#include <hzip/core/compressors/compressor_enums.h>
#include <hzip/utils/common.h>

struct HZ_MState : public rainman::context {
    uint8_t *data{};
    uint64_t length{};
    hzcodec::algorithms::ALGORITHM alg{};

    HZ_MState() {
        data = nullptr;
        length = 0;
        alg = hzcodec::algorithms::ALGORITHM::UNDEFINED;
    }

    [[nodiscard]] bool is_empty() const {
        return data == nullptr;
    }

    void destroy() {
        rfree(data);
    }
};

struct HZ_BlobHeader : public rainman::context {
    uint8_t *raw;
    uint64_t length;

    HZ_BlobHeader() {
        raw = nullptr;
        length = 0;
    }

    [[nodiscard]] bool is_empty() const {
        return raw == nullptr;
    }
};

struct HZ_Blob : public rainman::context {
    HZ_BlobHeader header{};
    HZ_MState *mstate;
    uint32_t *data;
    uint64_t size;
    uint8_t *o_data;
    uint64_t o_size;
    uint64_t mstate_id{};

    HZ_Blob() {
        mstate = nullptr;
        data = nullptr;
        o_data = nullptr;
        size = 0;
        o_size = 0;
    }

    void destroy() {
        rfree(data);
        rfree(o_data);
        rfree(header.raw);
    }
};

struct HZ_BlobSet {
    HZ_Blob *blobs = nullptr;
    uint64_t blob_count{};

    void destroy() const {
        for (uint64_t i = 0; i < blob_count; i++) {
            blobs[i].destroy();
        }
    }
};

#endif
