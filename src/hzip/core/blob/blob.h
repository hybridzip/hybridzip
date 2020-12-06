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
        alg = hzcodec::algorithms::ALGORITHM::UNCOMPRESSED;
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

// status: true indicates that the blob is compressed.

struct HZ_Blob : public rainman::context {
    HZ_BlobHeader header{};
    HZ_MState *mstate;
    uint8_t *data;
    uint64_t size;
    uint64_t o_size;
    uint64_t mstate_id{};
    bool status{};

    HZ_Blob() {
        mstate = nullptr;
        data = nullptr;
        size = 0;
        o_size = 0;
    }

    void evaluate(uint8_t *original_data) {
        status = size < o_size;
        if (!status) {
            data = original_data;
            size = o_size;

            // Destroy mstate if compression fails.
            // Replace it with a mstate with algorithm = UNCOMPRESSED
            if (mstate != nullptr) {
                mstate->destroy();
                rfree(mstate);
                mstate = rxnew(HZ_MState);
            }
        }
    }

    void destroy() {
        rfree(data);
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
